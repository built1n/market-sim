#include "globals.h"

/*** utility functions ***/

void quit_handler(struct player_t *player)
{
    (void) player;
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    atexit(cleanup);

#ifndef NDEBUG
    debug_init();
#endif

    curl_global_init(CURL_GLOBAL_DEFAULT);

    struct player_t *player = malloc(sizeof(struct player_t));
    memset(player, 0, sizeof(struct player_t));

    player->cash.cents = 1000000 * 100;

    while(1)
    {
        const struct command_t commands[] = {
            { "[B]uy", "buy", buy_handler },
            { "[S]ell", "sell", sell_handler },
            { "[P]rint portfolio", "print", print_handler },
            { "[U]pdate stock prices", "update", update_handler },
            { "Stock [i]nfo", "info", info_handler },
            { "[H]elp", "help", help_handler },
            { "[W]rite portfolio", "write", save_handler },
            { "[L]oad portfolio", "load", load_handler },
#ifndef NDEBUG
            { "[D]ebug menu", "debug", debug_handler },
#endif
            { "[Q]uit", "quit", quit_handler },
        };
        print_handler(player);
        printf("\n");

        do_menu(player, commands, ARRAYLEN(commands), "What would you like to do? ");
    }
}
