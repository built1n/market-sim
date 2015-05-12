#include "globals.h"

/* NOTE: integers are represented internally by unsigned long long ints, but in the save they are always 64 bits */

void save_handler(struct player_t *player)
{
    printf("Enter the file to save your portfolio in: ");

    char buf[128];
    scanf("%127s", buf);

    printf("Writing data...\n");
    FILE *f = fopen(buf, "w");

    const char *magic = "PORTv1";
    fwrite(magic, strlen(magic), 1, f);

    uint64_t be_cash = to_be64(player->cash.cents);

    fwrite(&be_cash, sizeof(be_cash), 1, f);

    for(uint i = 0; i < player->portfolio_len; ++i)
    {
        struct stock_t *stock = player->portfolio + i;

        uint64_t be_symlen = to_be64(strlen(stock->symbol));
        fwrite(&be_symlen, sizeof(be_symlen), 1, f);
        fwrite(stock->symbol, strlen(stock->symbol) + 1, 1, f);

        uint64_t be_count = to_be64(stock->count);
        fwrite(&be_count, sizeof(be_count), 1, f);
    }

    fclose(f);

    printf("Done saving.");
}
