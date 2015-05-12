#include <stdbool.h>
#include <stdint.h>

#define ARRAYLEN(x) (sizeof(x) / sizeof(x[0]))

typedef unsigned long long ullong;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

/* money is represented internally as cents */
struct money_t {
    ullong cents;
};

struct stock_t {
    char *symbol;
    char *fullname;
    ullong count;
    struct money_t current_price;
};

struct player_t {
    struct money_t cash;
    uint portfolio_len;
    struct stock_t *portfolio;
    bool need_to_free_portfolio;
};
