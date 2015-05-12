#include <stdbool.h>
#include <stdint.h>

#define ARRAYLEN(x) (sizeof(x) / sizeof(x[0]))

typedef uint64_t ullong;
typedef uint32_t ulong;
typedef uint32_t uint;
typedef uint16_t ushort;
typedef uint8_t uchar;

/* money is represented internally as cents */
struct money_t {
    ullong cents;
};

struct stock_t {
    char *symbol;
    char *fullname;
    ulong count;
    struct money_t current_price;
};

struct player_t {
    struct money_t cash;
    uint portfolio_len;
    struct stock_t *portfolio;
    bool need_to_free_portfolio;
};
