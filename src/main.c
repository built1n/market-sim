#include "globals.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

/*** utility functions ***/

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
                printf("%6s %30s %5llu  * $%5llu.%02llu = $%6llu.%02llu\n",
                       stock->symbol, stock->fullname, stock->count, stock->current_price.cents / 100, stock->current_price.cents % 100,
                       total_value / 100, total_value % 100);

                portfolio_value += stock->current_price.cents * stock->count;
            }
        }
        printf("================================================================================\n");

        printf("Portfolio value: $%llu.%02llu\n", portfolio_value / 100, portfolio_value % 100);

        printf("Current cash: $%llu.%02llu\n", player->cash.cents / 100, player->cash.cents % 100);

        ullong total = portfolio_value + player->cash.cents;
        printf("Total capital: $%llu.%02llu\n", total / 100, total % 100);

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
