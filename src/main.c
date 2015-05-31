#include "globals.h"

bool restricted = false;

/*** utility functions ***/

void quit_handler(struct player_t *player)
{
    (void) player;
    exit(EXIT_SUCCESS);
}

void quicksave_handler(struct player_t *player)
{
    if(restricted)
    {
        output("Saving forbidden in restricted mode.\n");
        return;
    }

    if(!player->filename)
    {
        save_handler(player);
        return;
    }

    output("Saving to '%s'.\n", player->filename);

    save_portfolio(player, player->filename);
}

int main(int argc, char *argv[])
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    struct player_t *player = malloc(sizeof(struct player_t));
    memset(player, 0, sizeof(struct player_t));

    char *save_file;
    char **save_file_p = &save_file;

    uint args_status = parse_args(argc, argv, save_file_p);

    if(args_status & ARG_FAILURE)
        return EXIT_FAILURE;

    if(!(args_status & ARG_NOCURSES) && !(args_status & ARG_BATCHMODE))
        curses_init();

    if(args_status & ARG_BATCHMODE)
        batch_init();

    atexit(cleanup);

    const struct sigaction handler = {
        .sa_handler = sig_handler
    };

    sigaction(SIGINT, &handler, NULL);

    heading("Market Simulator " PROGRAM_VERSION);

    if(args_status & ARG_RESTRICTED)
        restricted = true;

    if(args_status & ARG_HTML)
        html_out = true;

    if(args_status & ARG_LOADED)
    {
        /* save_file must be allocated with malloc(), make it so */
        char *filename = malloc(strlen(save_file) + 1);
        strcpy(filename, save_file);

        load_portfolio(player, filename);
    }
    else
        player->cash.cents = 1000 * 100;

    if(args_status & ARG_VERBOSE)
    {
        output("Verbose operation enabled.\n");
    }

    while(1)
    {
        const struct command_t commands[] = {
            { "[B]uy", "buy", buy_handler },
            { "[S]ell", "sell", sell_handler },
            { "[P]rint portfolio", "print", print_handler },
            { "[U]pdate stock prices", "update", update_handler },
            { "Stock [i]nfo", "info", info_handler },
            { "[L]oad portfolio from disk", "load", load_handler },
            { "[W]rite portfolio to disk", "write", save_handler },
            { "[Q]uick save", "quicksave", quicksave_handler },
            { "[E]xit", "exit", quit_handler },
        };

        do_menu(player, commands, ARRAYLEN(commands),
                "What would you like to do? ");
    }
}
