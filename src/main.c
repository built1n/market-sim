#include "globals.h"

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    while(*++str = toupper(*str));
}

void all_lower(char *str)
{
    while(*++str = tolower(*str));
}

size_t curl_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    struct data_buffer_t *buf = userdata;

    buf->data = realloc(buf->data, buf->back + size * nmemb);

    memcpy(buf->data + buf->back, ptr, size * nmemb);

    buf->back += size * nmemb;

    return CURLE_OK;
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

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);

    CURLcode res = curl_easy_perform(curl);

    /** now parse the data **/

    /* the stock name is in quotes, find it! */

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

void buy_handler(struct player_t *player)
{
    char sym[16];
    printf("Enter the ticker symbol of the stock you wish to purchase: ");
    scanf("%15s", sym);
    all_upper(sym);
    struct money_t price;
    printf("Getting stock information...\n");
    char *name;
    if(!get_stock_info(sym, &price, &name))
    {
        printf("Failed to get query information for '%s'\n", sym);
    }
    printf("Stock name: %s\n", name);
    printf("Price per share: $%d.%02d\n", price.cents / 100, price.cents % 100);
    printf("Enter the number of shares to be purchased (maximum %u): ", player->cash.cents / price.cents);
    ullong count = 0;
    scanf("%llu", &count);
    ullong cost = price.cents * count;

    if(cost > player->cash.cents)
    {
        printf("Not enough money!\n");
        return;
    }

    printf("This will cost $%d.%02d. Are you sure? ", cost / 100, cost % 100);
    char response[16];
    scanf("%15s", response);
    if(response[0] == 'Y' || response[0] == 'y')
    {
        printf("Confirmed.\n");
        player->portfolio_len += 1;
        player->portfolio = realloc(player->portfolio, player->portfolio_len);

        player->portfolio[player->portfolio_len - 1].symbol = strdup(sym);
        player->portfolio[player->portfolio_len - 1].fullname = strdup(name);
        player->portfolio[player->portfolio_len - 1].count = count;
        player->portfolio[player->portfolio_len - 1].current_price.cents = price.cents;

        player->cash.cents -= cost;
    }
    else
    {
        printf("Not confirmed.\n");
    }
}

void sell_handler(struct player_t *player)
{
}

void save_handler(struct player_t *player)
{
}

void update_handler(struct player_t *player)
{
    printf("Updating stock prices...\n");
    for(int i = 0; i < player->portfolio_len; ++i)
    {
        struct stock_t *stock = player->portfolio + i;
        get_stock_info(stock->symbol, &stock->current_price, &stock->fullname);
    }
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
        printf("Your portfolio:\n");
        printf("===============\n");
        for(int i = 0; i < player->portfolio_len; ++i)
        {
            struct stock_t *stock = player->portfolio + i;
            ullong total_value = stock->count * stock->current_price.cents;
            printf("%5s %30s %5d * $%d.%02d = $%d.%d\n",
                   stock->symbol, stock->fullname, stock->count, stock->current_price.cents / 100, stock->current_price.cents % 100,
                  total_value / 100, total_value % 100);
        }
        printf("===============\n");

        printf("Current cash: $%d.%02d\n", player->cash.cents / 100, player->cash.cents % 100);

        struct command_t {
            const char *name;
            const char *command;
            void (*handler)(struct player_t*);
        };

        const struct command_t commands[] = {
            { "[B]uy", "buy", buy_handler },
            { "[S]ell", "sell", sell_handler },
            { "[W]rite portfolio", "write", save_handler },
            { "[U]pdate stock prices", "update", update_handler },
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
