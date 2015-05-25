#include "globals.h"

/* for htnos/htnol */
#include <arpa/inet.h>

void cleanup(void)
{
    curl_global_cleanup();
    endwin();
}

void sig_handler(int sig)
{
    (void) sig;
    cleanup();
    exit(EXIT_FAILURE);
}

struct data_buffer_t {
    char *data;
    uint back;
};

void all_upper(char *str)
{
    while(*str)
    {
        *str = toupper(*str);
        ++str;
    }
}

void all_lower(char *str)
{
    while(*str)
    {
        *str = tolower(*str);
        ++str;
    }
}

size_t download_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct data_buffer_t *buf = userdata;

    buf->data = realloc(buf->data, buf->back + size * nmemb);

    memcpy(buf->data + buf->back, ptr, size * nmemb);

    buf->back += size * nmemb;

    return size * nmemb;
}

bool get_stock_info(char *symbol, struct money_t *price, char **name_ret)
{
    CURL *curl = curl_easy_init();
    if(!curl)
    {
        return false;
    }

    char url[256];
    snprintf(url, sizeof(url), "http://download.finance.yahoo.com/d/quotes.csv?s=%s&f=nl1&e=.csv", symbol);

    struct data_buffer_t buf;
    memset(&buf, 0, sizeof(buf));

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(res != CURLE_OK || buf.data[0] != '"')
    {
        fail("Failed to query information for '%s'.", symbol);
    }

    /* null-terminate buffer */
    buf.data = realloc(buf.data, buf.back + 1);
    buf.data[buf.back] = '\0';

    /** now parse the data **/

    char *p1 = buf.data;
    char ** ptr = &p1;

    *name_ret = csv_read(ptr);

    char *pricebuf = csv_read(ptr);
    ullong dollars, cents;

    /* dirty hack! */
    sscanf(pricebuf, "%llu.%2llu", &dollars, &cents);
    free(pricebuf);

    price->cents = dollars * 100 + cents;

    free(buf.data);
    return true;
}

int compare_stocks(const void *a, const void *b)
{
    const struct stock_t *a1 = a, *b1 = b;
    return strcmp(a1->symbol,
                  b1->symbol);
}

static enum { OTHER = 0, LITTLE = 1, BIG = 2 } endianness;

static void detect_endianness(void)
{
    ulong test = 0x12345678;
    uchar *ptr = (uchar*)&test;
    if(*ptr == 0x12)
        endianness = BIG;
    else if(*ptr == 0x78)
        endianness = LITTLE;
    else
    {
        fail("Failed to detect system endianness.");
    }
}

uint64_t to_be64(uint64_t n)
{
    if(!endianness)
    {
        detect_endianness();
    }

    if(endianness == LITTLE)
    {
        n = (n & 0x00000000FFFFFFFF) << 32 | (n & 0xFFFFFFFF00000000) >> 32;
        n = (n & 0x0000FFFF0000FFFF) << 16 | (n & 0xFFFF0000FFFF0000) >> 16;
        n = (n & 0x00FF00FF00FF00FF) << 8  | (n & 0xFF00FF00FF00FF00) >> 8;
    }
    return n;
}

uint64_t to_sys64(uint64_t n)
{
    if(!endianness)
    {
        detect_endianness();
    }

    if(endianness == BIG)
        return n;
    else
        return to_be64(n);
}

uint32_t to_be32(uint32_t n)
{
    return htonl(n);
}

uint32_t to_sys32(uint32_t n)
{
    return ntohl(n);
}

uint16_t to_be16(uint16_t n)
{
    return htons(n);
}

uint16_t to_sys16(uint16_t n)
{
    return ntohs(n);
}

struct stock_t *find_stock(struct player_t *player, char *sym)
{
    for(uint i = 0; i < player->portfolio_len; ++i)
    {
        if(strcmp(player->portfolio[i].symbol, sym) == 0)
        {
            return player->portfolio + i;
        }
    }

    return NULL;
}

void print_handler(struct player_t *player)
{
    heading("Your Portfolio");

    ullong portfolio_value = 0;

    if(player->portfolio_len == 0)
    {
        output(" < EMPTY >\n");
    }
    else
    {
        for(uint i = 0; i < player->portfolio_len; ++i)
        {
            struct stock_t *stock = player->portfolio + i;
            if(stock->count)
            {
                ullong total_value = stock->count * stock->current_price.cents;
                output("%6s %40s %5llu  * $%5llu.%02llu = $%6llu.%02llu\n",
                       stock->symbol, stock->fullname, stock->count, stock->current_price.cents / 100, stock->current_price.cents % 100,
                       total_value / 100, total_value % 100);

                portfolio_value += stock->current_price.cents * stock->count;
            }
        }
    }
    horiz_line();

    output("Portfolio value: $%llu.%02llu\n", portfolio_value / 100, portfolio_value % 100);

    output("Cash balance: $%llu.%02llu\n", player->cash.cents / 100, player->cash.cents % 100);

    ullong total = portfolio_value + player->cash.cents;

    output("Total capital: $%llu.%02llu\n", total / 100, total % 100);
}

static char *read_string_curses(void)
{
    char *ret = malloc(1);
    size_t len = 1;

    ret[0] = '\0';

    char c;
    do {
        c = getch();
        if(c != '\b' && c != '\n')
        {
            ++len;
            ret = realloc(ret, len);
            ret[len - 1] = '\0';
            ret[len - 2] = c;
        }
        output("%d\n", c);
        sleep(1);
    } while(c != '\n' && c != ERR);

    return ret;
}

static char *read_string_nocurses(void)
{
    char *ret = NULL;
    size_t len = 0;

    len = getline(&ret, &len, stdin);
    if(len)
        ret[len - 1] = '\0';

    if(len == (size_t) -1)
    {
        free(ret);
        fail("Encountered end-of-file.");
    }
    return ret;
}

char* (*read_string)(void) = read_string_nocurses;

char *read_ticker(void)
{
    char *str = read_string();
    all_upper(str);
    return str;
}

ullong read_int(void)
{
    char *str = read_string();
    ullong ret = -1;
    sscanf(str, "%llu", &ret);

    free(str);

    return ret;
}

void update_handler(struct player_t *player)
{
    output("Updating stock prices...\n");
    for(uint i = 0; i < player->portfolio_len; ++i)
    {
        struct stock_t *stock = player->portfolio + i;
        if(stock->count > 0)
        {
            output("%s...\n", stock->symbol);
            get_stock_info(stock->symbol, &stock->current_price, &stock->fullname);
        }
    }
}

uint parse_args(int argc, char *argv[], char **port_file)
{
    uint ret = 0;

    for(int i = 1; i < argc; ++i)
    {
        char *arg = argv[i];
        if(arg && arg[0] != '\0')
        {
            if(arg[0] == '-')
            {
                if(strcmp(arg, "--batch") == 0)
                {
                    ret |= ARG_BATCHMODE;
                }
                else if(strcmp(arg, "--help") == 0 ||
                        strcmp(arg, "-h")     == 0)
                {
                    print_usage(argc, argv);
                    ret |= ARG_FAILURE;
                    break;
                }
                else if(strcmp(arg, "--nocurses") == 0)
                {
                    ret |= ARG_NOCURSES;
                }
                else if(strcmp(arg, "-r") == 0 ||
                        strcmp(arg, "--restrict") == 0)
                {
                    ret |= ARG_RESTRICTED;
                }
                else if(strcmp(arg, "-v") == 0 ||
                        strcmp(arg, "--verbose") == 0)
                {
                    ret |= ARG_VERBOSE;
                }
                else if(strcmp(arg, "--version") == 0)
                {
                    print_version();
                    ret |= ARG_FAILURE;
                    break;
                }
                else if(strcmp(arg, "--") == 0)
                {
                    break;
                }
                else
                {
                    output("Unrecognized option '%s'\n", arg, argv[0]);
                    print_usage(argc, argv);
                    ret |= ARG_FAILURE;
                }
            }
            else
            {
                if(!(ret & ARG_LOADED))
                {
                    *port_file = arg;
                    ret |= ARG_LOADED;
                }
                else
                {
                    output("FATAL: multiple portfolio files specified.\n");
                    ret |= ARG_FAILURE;
                    break;
                }
            }
        }

    }

    if(ret & ARG_FAILURE)
        return ret;

    return ret;
}

void fail(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    cleanup();

    fprintf(stdout, "FATAL: %s\n", buf);

    exit(EXIT_FAILURE);
}

static int curses_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int ret = vwprintw(stdscr, fmt, ap);

    refresh();

    va_end(ap);

    return ret;
}

int (*output)(const char*, ...) = printf;

void horiz_line_curses(void)
{
    for(int i = 0; i < getmaxx(stdscr); ++i)
    {
        output("=");
    }
}

void horiz_line_nocurses(void)
{
    struct winsize sz;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &sz);
    for(int i = 0; i < sz.ws_col; ++i)
        output("=");
    output("\n");
}

void (*horiz_line)(void) = horiz_line_nocurses;

void heading_curses(const char *fmt, ...)
{
    char text[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);

    int len = strlen(text) / 2;
    int beg_x = getmaxx(stdscr) / 2 - len;
    int d = 0;
    if(strlen(text) & 1)
        d++;

    for(int i = 0; i < beg_x - 1; ++i)
        output("=");
    output(" ");
    output(text);
    output(" ");
    for(int i = 0; i < getmaxx(stdscr) - getmaxx(stdscr) / 2 - len - 1 - d; ++i)
        output("=");
}

void heading_nocurses(const char *fmt, ...)
{
    char text[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(text, sizeof(text), fmt, ap);
    va_end(ap);

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int len = strlen(text) / 2;
    int beg_x = w.ws_col / 2 - len;
    int d = 0;
    if(strlen(text) & 1)
        d++;

    for(int i = 0; i < beg_x - 1; ++i)
        output("=");
    output(" ");
    output(text);
    output(" ");
    for(int i = 0; i < w.ws_col / 2 - len - 1 - d; ++i)
        output("=");
    output("\n");
}

void (*heading)(const char*, ...) = heading_nocurses;

bool have_color = false;

void use_color(int col)
{
    if(have_color)
    {
        attron(COLOR_PAIR(col));
    }
}

void stop_color(int col)
{
    if(have_color)
    {
        attroff(COLOR_PAIR(col));
    }
}

void curses_init(void)
{
    initscr();
    echo();
    nocbreak();
    nl();
    scrollok(stdscr, true);
    output = curses_printf;
    read_string = read_string_curses;
    horiz_line = horiz_line_curses;
    heading = heading_curses;

    if(has_colors())
    {
        have_color = true;
        start_color();
        attron(A_BOLD);
        init_color(COLOR_WHITE, 1000, 1000, 1000);
        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        use_color(COL_NORM);
    }
    else
    {
        have_color = false;
    }
}

int dummy_output(const char* f, ...)
{
    (void) f;
    return 0;
}

void batch_init(void)
{
    output = dummy_output;
}
