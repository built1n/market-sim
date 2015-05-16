#include "globals.h"

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

    print_handler(player);

    while(1)
    {
        struct command_t {
            const char *name;
            const char *command;
            void (*handler)(struct player_t*);
        };

        const struct command_t commands[] = {
            { "[B]uy", "buy", buy_handler },
            { "[S]ell", "sell", sell_handler },
            { "[P]rint portfolio", "print", print_handler },
            { "[U]pdate stock prices", "update", update_handler },
            { "Stock [i]nfo", "info", info_handler },
            { "[W]rite portfolio", "write", save_handler },
            { "[L]oad portfolio", "load", load_handler },
            { "[Q]uit", "quit", quit_handler },
        };

        for(uint i = 0; i < ARRAYLEN(commands); ++i)
        {
            printf("%d. %s\n", i + 1, commands[i].name);
        }

        printf("What would you like to do? ");
        char *cmdbuf = NULL;
        size_t cmdlen = 0;
        cmdlen = getline(&cmdbuf, &cmdlen, stdin);

        all_lower(cmdbuf);

        cmdbuf[cmdlen - 1] = '\0';

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

        printf("\n");
    }
}
