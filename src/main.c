#include "globals.h"

/*** utility functions ***/

void quit_handler(struct player_t *player)
{
    (void) player;
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    atexit(cleanup);

    curl_global_init(CURL_GLOBAL_DEFAULT);

    struct player_t *player = malloc(sizeof(struct player_t));
    memset(player, 0, sizeof(struct player_t));

    uint args_status = parse_args(player, argc, argv);

    if(args_status & ARG_FAILURE)
        return EXIT_FAILURE;

    if(!args_status & ARG_LOADED)
        player->cash.cents = 1000 * 100;

    while(1)
    {
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

        do_menu(player, commands, ARRAYLEN(commands), "What would you like to do? ");
    }
}
