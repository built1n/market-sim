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
    struct money_t price;
    printf("Getting stock information...\n");
    char *name;
    if(!get_stock_info(sym, &price, &name))
    {
        printf("Failed to get query information for '%s'\n", sym);
    }
    printf("Stock name: %s\n", name);
    printf("Price per share: $%d.%d\n", price.cents / 100, price.cents % 100);
}

void sell_handler(struct player_t *player)
{
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

    while(1)
    {
        printf("Your portfolio:\n");
        printf("===============\n");
        for(int i = 0; i < player->portfolio_len; ++i)
        {
            struct stock_t *stock = player->portfolio + i;
            printf("%5s %20s $%d.%d\n", stock->symbol, stock->fullname, stock->price.cents / 100, stock->price.cents % 100);
        }
        printf("===============\n");

        printf("Current cash: $%d.%d\n", player->cash.cents / 100, player->cash.cents % 100);

        struct command_t {
            const char *name;
            const char *command;
            void (*handler)(struct player_t*);
        };

        const struct command_t commands[] = {
            { "[B]uy", "buy", buy_handler },
            { "[S]ell", "sell", sell_handler },
            { "[Q]uit", "quit", quit_handler }
        };

        for(uint i = 0; i < ARRAYLEN(commands); ++i)
        {
            printf("%d. %s\n", i + 1, commands[i].name);
        }

        printf("What would you like to do? ");
        char cmdbuf[32];
        scanf("%31s", cmdbuf);

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
