#include "globals.h"

#include <curl/curl.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*** prototypes ***/

void buy_handler(struct player_t*);
void sell_handler(struct player_t*);
void update_handler(struct player_t*);
void save_handler(struct player_t*);
void load_handler(struct player_t*);
void quit_handler(struct player_t*);

/*** utility functions ***/

void cleanup(void)
{
    curl_global_cleanup();
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

    curl_easy_setopt(curl, CURLOPT_URL, url);

    struct data_buffer_t buf;
    memset(&buf, 0, sizeof(buf));

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_callback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    /** now parse the data **/

    /* the stock name is in quotes, find it! */

    /* check for validity */
    if(buf.back == 0 || buf.data[0] != '"' || res != CURLE_OK)
    {
        printf("Failed to retrieve stock data.\n");
        if(res != CURLE_OK)
        {
            printf("Download library error (%d): '%s'\n", res, curl_easy_strerror(res));
        }
        return false;
    }

    uint name_len = 0;
    for(uint i = 1; i < buf.back; ++i)
    {
        if(buf.data[i] == '"')
            break;
        ++name_len;
    }

    const uint name_offs = 1;
    uint price_offs = name_len + 3;
    uint price_len = buf.back - price_offs;

    char *name = malloc(name_len + 1);
    memcpy(name, buf.data + name_offs, name_len);
    name[name_len] = '\0';

    *name_ret = name;

    /* get price */

    char *pricebuf = malloc(price_len + 1);
    memcpy(pricebuf, buf.data + price_offs, price_len);
    pricebuf[price_len] = '\0';

    free(buf.data);

    /* remove the decimal point */

    for(int i = 0; i < price_len; ++i)
    {
        if(pricebuf[i] == '.')
            memmove(pricebuf + i, pricebuf + i + 1, price_len - i);
    }

    price->cents = strtoull(pricebuf, NULL, 10);

    free(pricebuf);

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
        printf("FATAL: failed to detect system endianness!\n");
        exit(EXIT_FAILURE);
    }
}

ullong to_be64(ullong n)
{
    if(!endianness)
    {
        detect_endianness();
    }

    if(BIG)
    {
        n = (n & 0x00000000FFFFFFFF) << 32 | (n & 0xFFFFFFFF00000000) >> 32;
        n = (n & 0x0000FFFF0000FFFF) << 16 | (n & 0xFFFF0000FFFF0000) >> 16;
        n = (n & 0x00FF00FF00FF00FF) << 8  | (n & 0xFF00FF00FF00FF00) >> 8;
    }
    return n;
}

ullong to_sys64(ullong n)
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

/*** driver functions ***/

void buy_handler(struct player_t *player)
{
    char *sym = malloc(16);
    printf("Enter the ticker symbol of the stock you wish to purchase: ");
    scanf("%15s", sym);
    all_upper(sym);

    struct money_t price;

    printf("Getting stock information...\n");

    char *name;

    if(!get_stock_info(sym, &price, &name))
    {
        printf("Failed to get query information for '%s'.\n", sym);
        return;
    }

    printf("Stock name: %s\n", name);
    printf("Price per share: $%d.%02d\n", price.cents / 100, price.cents % 100);
    printf("Enter the number of shares to be purchased (maximum %u): ", player->cash.cents / price.cents);

    ullong count = 0;

    scanf("%llu", &count);

    if(count <= 0)
    {
        printf("Purchase cancelled.\n");
        return;
    }

    ullong cost = price.cents * count;

    if(cost > player->cash.cents)
    {
        printf("Not enough money!\n");
        return;
    }

    printf("This will cost $%d.%02d. Are you sure? ", cost / 100, cost % 100);
    char response[16];
    scanf("%15s", response);
    all_lower(response);

    if(response[0] == 'y')
    {
        printf("Confirmed.\n");

        /* add the stock to the portfolio or increase the count of a stock */

        for(uint i = 0; i < player->portfolio_len; ++i)
        {
            if(strcmp(player->portfolio[i].symbol, sym) == 0)
            {
                struct stock_t *stock = player->portfolio + i;
                stock->count += count;
                stock->current_price.cents = price.cents;
                goto done;
            }
        }

        /* not found, add a new one */
        player->portfolio_len += 1;
        player->portfolio = realloc(player->portfolio, player->portfolio_len * sizeof(struct stock_t));
        player->need_to_free_portfolio = true;

        printf("sym: %s\n", sym);
        printf("name: %s\n", name);
        player->portfolio[player->portfolio_len - 1].symbol = sym;
        player->portfolio[player->portfolio_len - 1].fullname = name;
        player->portfolio[player->portfolio_len - 1].count = count;
        player->portfolio[player->portfolio_len - 1].current_price.cents = price.cents;
    done:

        player->cash.cents -= cost;

        /* sort the portfolio alphabetically by ticker symbol */
        qsort(player->portfolio, player->portfolio_len, sizeof(struct stock_t), compare_stocks);
    }
    else
    {
        printf("Not confirmed.\n");
    }
}

void sell_handler(struct player_t *player)
{
    char *sym = malloc(16);
    printf("Enter the ticker symbol of the stock you wish to sell: ");
    scanf("%15s", sym);
    all_upper(sym);

    printf("Getting stock information...\n");

    struct stock_t *stock = NULL;
    uint stock_idx;

    for(stock_idx = 0; stock_idx < player->portfolio_len; ++stock_idx)
    {
        if(strcmp(player->portfolio[stock_idx].symbol, sym) == 0)
        {
            stock = player->portfolio + stock_idx;
            break;
        }
    }

    if(!stock)
    {
        printf("Couldn't find '%s' in portfolio.\n", sym);
        return;
    }

    update_handler(player);

    printf("You currently own %d shares of '%s' (%s) valued at $%d.%02d each.\n", stock->count, stock->fullname, stock->symbol, stock->current_price.cents / 100, stock->current_price.cents % 100);

    ullong sell = 0;
    printf("How many shares do you wish to sell? ");
    scanf("%llu", &sell);

    if(!sell)
    {
        printf("Sale cancelled.\n");
        return;
    }

    if(stock->count < sell)
    {
        printf("You don't own enough shares!\n");
        return;
    }

    ullong sell_total = stock->current_price.cents * sell;

    printf("This will sell %d shares for $%d.%02d total. Proceed? ", sell, sell_total / 100, sell_total % 100);

    char response[16];
    scanf("%15s", response);
    all_lower(response);

    if(response[0] == 'y')
    {
        stock->count -= sell;

        if(stock->count == 0)
        {
            /* remove this item from the portfolio */
            memmove(player->portfolio + stock_idx, player->portfolio + stock_idx + 1, sizeof(struct stock_t) * (player->portfolio_len - stock_idx - 1));
            player->portfolio_len -= 1;
            player->portfolio = realloc(player->portfolio, sizeof(struct stock_t) * player->portfolio_len);
        }

        player->cash.cents += sell_total;

        printf("%d shares sold for $%d.%02d total.\n", sell, sell_total / 100, sell_total % 100);
    }
    else
    {
        printf("Not confirmed.\n");
    }
}

void save_handler(struct player_t *player)
{
    printf("Enter the file to save your portfolio in: ");

    char buf[128];
    scanf("%127s", buf);

    printf("Writing data...\n");
    FILE *f = fopen(buf, "w");

    const char *magic = "PORTv1";
    fwrite(magic, strlen(magic), 1, f);

    ullong be_cash = to_be64(player->cash.cents);

    fwrite(&be_cash, sizeof(be_cash), 1, f);

    for(uint i = 0; i < player->portfolio_len; ++i)
    {
        struct stock_t *stock = player->portfolio + i;

        ullong be_symlen = to_be64(strlen(stock->symbol));
        fwrite(&be_symlen, sizeof(be_symlen), 1, f);
        fwrite(stock->symbol, strlen(stock->symbol) + 1, 1, f);

        ullong be_count = to_be64(stock->count);
        fwrite(&be_count, sizeof(be_count), 1, f);
    }

    fclose(f);

    printf("Done saving.");
}

void update_handler(struct player_t *player)
{
    printf("Updating stock prices...\n");
    for(int i = 0; i < player->portfolio_len; ++i)
    {
        struct stock_t *stock = player->portfolio + i;
        printf("%s...\n", stock->symbol);
        get_stock_info(stock->symbol, &stock->current_price, &stock->fullname);
    }
}

void load_handler(struct player_t *player)
{
    printf("Enter the file to load portfolio from: ");
    char buf[128];
    scanf("%127s", buf);

    printf("Loading portfolio...\n");

    if(player->need_to_free_portfolio)
        free(player->portfolio);
    player->portfolio_len = 0;

    FILE *f = fopen(buf, "r");
    char magic[6];
    if(!f || fread(magic, 1, sizeof(magic), f) != 6 || memcmp(magic, "PORTv1", sizeof(magic)) != 0)
    {
        printf("FATAL: Failed to load save.");
        exit(EXIT_FAILURE);
    }

    ullong cash;
    if(fread(&cash, sizeof(cash), 1, f) != 1)
    {
        printf("cash: %llu\n", cash);
        printf("FATAL: Failed to load save.");
        exit(EXIT_FAILURE);
    }
    cash = to_sys64(cash);

    player->cash.cents = cash;

    fflush(stdout);
    do {
        /* read portfolio data */

        player->portfolio_len += 1;
        player->portfolio = realloc(player->portfolio, player->portfolio_len * sizeof(struct stock_t));
        player->need_to_free_portfolio = true;

        ullong symlen;
        if(fread(&symlen, sizeof(symlen), 1, f) != 1)
        {
            printf("FATAL: Save is corrupted (symbol length too short).\n");
            exit(EXIT_FAILURE);
        }
        symlen = to_sys64(symlen);
        char *sym = malloc(symlen + 1);
        if(fread(sym, symlen + 1, 1, f) != 1)
        {
            printf("FATAL: Save is corrupted (symbol too short).\n");
            exit(EXIT_FAILURE);
        }

        player->portfolio[player->portfolio_len - 1].symbol = sym;

        ullong count;
        if(fread(&count, sizeof(count), 1, f) != 1)
        {
            printf("FATAL: Save is corrupted (count too short).\n");
            exit(EXIT_FAILURE);
        }
        count = to_sys64(count);

        player->portfolio[player->portfolio_len - 1].count = count;

        int junk = fgetc(f);
        ungetc(junk, f);

    } while (!feof(f) && !ferror(f));

    update_handler(player);
}

void quit_handler(struct player_t *player)
{
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    atexit(cleanup);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    struct player_t *player = malloc(sizeof(struct player_t));
    memset(player, 0, sizeof(struct player_t));

    player->cash.cents = 1000000 * 100;

    update_handler(player);

    while(1)
    {
        printf("\nYour portfolio:\n");
        printf("================================================================================\n");

        ullong portfolio_value = 0;

        if(player->portfolio_len == 0)
        {
            printf(" < EMPTY >\n");
        }
        else
        {
            for(int i = 0; i < player->portfolio_len; ++i)
            {
                struct stock_t *stock = player->portfolio + i;
                ullong total_value = stock->count * stock->current_price.cents;
                printf("%6s %30s %5d * $%5d.%02d = $%6d.%02d\n",
                       stock->symbol, stock->fullname, stock->count, stock->current_price.cents / 100, stock->current_price.cents % 100,
                       total_value / 100, total_value % 100);

                portfolio_value += stock->current_price.cents * stock->count;
            }
        }
        printf("================================================================================\n");

        printf("Portfolio value: $%d.%02d\n", portfolio_value / 100, portfolio_value % 100);

        printf("Current cash: $%d.%02d\n", player->cash.cents / 100, player->cash.cents % 100);

        ullong total = portfolio_value + player->cash.cents;
        printf("Total capital: $%d.%02d\n", total / 100, total % 100);

        struct command_t {
            const char *name;
            const char *command;
            void (*handler)(struct player_t*);
        };

        const struct command_t commands[] = {
            { "[B]uy", "buy", buy_handler },
            { "[S]ell", "sell", sell_handler },
            { "[U]pdate stock prices", "update", update_handler },
            { "[W]rite portfolio", "write", save_handler },
            { "[L]oad portfolio", "load", load_handler },
            { "[Q]uit", "quit", quit_handler },
        };

        for(uint i = 0; i < ARRAYLEN(commands); ++i)
        {
            printf("%d. %s\n", i + 1, commands[i].name);
        }

        printf("What would you like to do? ");
        char cmdbuf[32];
        scanf("%31s", cmdbuf);

        all_lower(cmdbuf);
        /* find the best command */

        int best_command = -1;

        /* first, search for an exact match */
        for(int i = 0; i < ARRAYLEN(commands); ++i)
        {
            if(strcmp(cmdbuf, commands[i].command) == 0)
            {
                best_command = i;
                goto exec_cmd;
            }
        }

        /* now look for a partial match */
        for(int i = 0; i < ARRAYLEN(commands); ++i)
        {
            int len = strlen(cmdbuf);
            if(len > strlen(commands[i].command))
                continue;
            for(int j = 1; j <= len; ++j)
            {
                char *buf1 = malloc(j + 1);
                memset(buf1, 0, j + 1);
                memcpy(buf1, cmdbuf, j);
                buf1[j] = '\0';

                char *buf2 = malloc(j + 1);
                memset(buf2, 0, j + 1);
                memcpy(buf2, commands[i].command, j);
                buf2[j] = '\0';

                if(strcmp(buf1, buf2) == 0)
                {
                    best_command = i;
                    free(buf1);
                    free(buf2);
                    goto exec_cmd;
                }
                else
                {
                    free(buf1);
                    free(buf2);
                }
            }
        }

    exec_cmd:

        if(best_command >= 0)
            commands[best_command].handler(player);
    }
}
