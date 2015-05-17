#include "globals.h"

/* NOTE: integers are represented internally by unsigned long long ints, but in the save they are always 64 bits */

#define FAIL() exit(EXIT_FAILURE);

uint64_t read_be64(FILE *f)
{
    uint64_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();
    return to_sys64(n);
}

uint32_t read_be32(FILE *f)
{
    uint32_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();
    return to_sys32(n);
}

uint16_t read_be16(FILE *f)
{
    uint16_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();
    return to_sys16(n);
}

uint8_t read_int8(FILE *f)
{
    uint8_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();
    return n;
}

void load_handler(struct player_t *player)
{
    printf("Enter the file to load portfolio from: ");
    char *filename = read_string();

    printf("Loading portfolio...\n");

    if(player->need_to_free_portfolio)
        free(player->portfolio);
    player->portfolio_len = 0;

    FILE *f = fopen(filename, "rb");
    free(filename);

    char magic[6];
    if(!f || fread(magic, 1, sizeof(magic), f) != 6 || memcmp(magic, "PORTv2", sizeof(magic)) != 0)
    {
        printf("FATAL: Failed to load save.\n");
        exit(EXIT_FAILURE);
    }

    player->cash.cents = read_be64(f);

    do {
        /* read portfolio data */

        player->portfolio_len += 1;
        player->portfolio = realloc(player->portfolio, player->portfolio_len * sizeof(struct stock_t));
        player->need_to_free_portfolio = true;

        struct stock_t *stock = player->portfolio + player->portfolio_len - 1;

        memset(stock, 0, sizeof(struct stock_t));

        uint64_t symlen = read_be64(f);
        char *sym = malloc(symlen + 1);
        if(fread(sym, symlen + 1, 1, f) != 1)
        {
            printf("FATAL: Save is corrupted (symbol too short).\n");
            exit(EXIT_FAILURE);
        }

        stock->symbol = sym;

        stock->count = read_be64(f);

        uint32_t histlen = read_be32(f);

        /* load history */

        if(histlen)
        {
            stock->history = malloc(histlen * sizeof(struct history_item));

            struct history_item *hist = stock->history;

            for(uint i = 0; i < histlen; ++i)
            {
                hist->action = read_be32(f);
                hist->count = read_be64(f);
                hist->price.cents = read_be64(f);

                hist->action_time.year = read_be16(f);
                hist->action_time.month = read_int8(f);
                hist->action_time.day = read_int8(f);
                hist->action_time.hour = read_int8(f);
                hist->action_time.minute = read_int8(f);
                hist->action_time.second = read_int8(f);

                if(i + 1 < histlen)
                    hist->next = hist + 1;
                else
                    hist->next = NULL;

                ++stock->history_len;
                hist = hist->next;
            }
        }
        int junk = fgetc(f);
        ungetc(junk, f);

    } while (!feof(f) && !ferror(f));

    update_handler(player);
}
