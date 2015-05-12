#include "globals.h"

/* NOTE: integers are represented internally by unsigned long long ints, but in the save they are always 64 bits */

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

    uint64_t cash;
    if(fread(&cash, sizeof(cash), 1, f) != 1)
    {
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

        uint64_t symlen;
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

        uint64_t count;
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
