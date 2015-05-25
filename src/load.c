#include "globals.h"

/* NOTE: integers are represented internally by unsigned long long ints, but in the save they are always 64 bits */

static uint32_t cksum;

#define ADD_CKSUM(x) (cksum += (x*x) + 1)

#define FAIL() fail("Failed to load save: Unexpected end-of-file")

uint64_t read_be64(FILE *f)
{
    uint64_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();

    n = to_sys64(n);

    ADD_CKSUM(n);

    return n;
}

uint32_t read_be32(FILE *f)
{
    uint32_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();

    n = to_sys32(n);

    ADD_CKSUM(n);

    return n;
}

uint32_t read_be32_nocheck(FILE *f)
{
    uint32_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();

    n = to_sys32(n);

    return n;
}

uint16_t read_be16(FILE *f)
{
    uint16_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();

    n = to_sys16(n);

    ADD_CKSUM(n);

    return n;
}

uint8_t read_int8(FILE *f)
{
    uint8_t n;
    if(fread(&n, sizeof(n), 1, f) != 1)
        FAIL();

    ADD_CKSUM(n);

    return n;
}

/* this is SLOWWW, but who cares? :P */
size_t ck_read(char *buf, size_t sz, size_t nmemb, FILE* f)
{
    for(size_t i = 0 ; i < sz * nmemb; ++i)
    {
        buf[i] = read_int8(f);
    }

    return nmemb;
}

void load_portfolio(struct player_t *player, const char *filename)
{
    output("Loading portfolio...\n");

    if(player->need_to_free_portfolio)
        free(player->portfolio);

    player->portfolio_len = 0;
    player->portfolio = NULL;

    cksum = 0;

    FILE *f = fopen(filename, "rb");

    char magic[MAGIC_LEN];
    if(!f)
    {
        fail("Failed to load save: %s", strerror(errno));
    }

    if(ck_read(magic, 1, sizeof(magic), f) != 6 || memcmp(magic, SAVE_MAGIC, sizeof(magic)) != 0)
    {
        fail("Failed to load save: Invalid file signature");
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
        if(ck_read(sym, symlen + 1, 1, f) != 1)
        {
            fail("Failed to load save: Unexpected end-of-file");
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

        uint32_t ck = read_be32_nocheck(f);

        if(ck != cksum)
        {
            fail("Failed to load save: Bad checksum", ck, cksum);
        }

        int junk = fgetc(f);
        ungetc(junk, f);

    } while (!feof(f) && !ferror(f));

    update_handler(player);
}

void load_handler(struct player_t *player)
{
    if(restricted)
    {
        output("Forbidden.\n");
        return;
    }
    output("Enter the file to load portfolio from: ");
    char *filename = read_string();

    load_portfolio(player, filename);

    free(filename);
}
