#include "globals.h"

/* NOTE: integers are represented internally as long long ints, but in the save they are always 64 bits */

static uint64_t cksum;

#define ADD_CKSUM(x) (cksum += (x*x) + 1)

static bool write_be64(FILE *f, uint64_t n)
{
    ADD_CKSUM(n);

    n = to_be64(n);
    if(fwrite(&n, sizeof(n), 1, f) != 1)
        return false;
    return true;
}

static bool write_be32(FILE *f, uint32_t n)
{
    ADD_CKSUM(n);

    n = to_be32(n);
    if(fwrite(&n, sizeof(n), 1, f) != 1)
        return false;
    return true;
}

static bool write_be32_noupdate(FILE *f, uint32_t n)
{
    n = to_be32(n);
    if(fwrite(&n, sizeof(n), 1, f) != 1)
        return false;
    return true;
}

static bool write_be16(FILE *f, uint16_t n)
{
    ADD_CKSUM(n);

    n = to_be16(n);
    if(fwrite(&n, sizeof(n), 1, f) != 1)
        return false;
    return true;
}

static bool write_int8(FILE *f, uint8_t n)
{
    ADD_CKSUM(n);

    if(fwrite(&n, sizeof(n), 1, f) != 1)
        return false;
    return true;
}

size_t ck_write(const char *buf, size_t sz, size_t nmemb, FILE *f)
{
    for(size_t i = 0 ; i < sz * nmemb; ++i)
    {
        write_int8(f, buf[i]);
    }

    return nmemb;
}

void save_handler(struct player_t *player)
{
    printf("Enter the file to save your portfolio in: ");

    char *filename = read_string();

    printf("Writing data...\n");
    FILE *f = fopen(filename, "wb");

    free(filename);

    cksum = 0;

    const char *magic = "PORTv2";
    ck_write(magic, strlen(magic), 1, f);

    write_be64(f, player->cash.cents);

    for(uint i = 0; i < player->portfolio_len; ++i)
    {
        struct stock_t *stock = player->portfolio + i;

        write_be64(f, strlen(stock->symbol));

        ck_write(stock->symbol, strlen(stock->symbol) + 1, 1, f);

        write_be64(f, stock->count);

        write_be32(f, stock->history_len);

        /* write history */
        struct history_item *hist = stock->history;
        while(hist)
        {
            write_be32(f, hist->action);
            write_be64(f, hist->count);
            write_be64(f, hist->price.cents);

            write_be16(f, hist->action_time.year);
            write_int8(f, hist->action_time.month);
            write_int8(f, hist->action_time.day);
            write_int8(f, hist->action_time.hour);
            write_int8(f, hist->action_time.minute);
            write_int8(f, hist->action_time.second);

            hist = hist->next;
        }

        write_be32_noupdate(f, cksum);
    }

    fclose(f);

    printf("Done saving.\n");
}
