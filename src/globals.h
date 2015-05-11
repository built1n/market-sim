#include <stdbool.h>

#define ARRAYLEN(x) (sizeof(x) / sizeof(x[0]))

typedef unsigned long long ullong;
typedef unsigned long ulong;
typedef unsigned uint;
typedef unsigned short ushort;
typedef unsigned char uchar;

struct date_t {
    ushort year;
    ushort day; /* 0 = 1st jan, 364 = 31st dec */
};

/* money is represented internally as dollars + cents */
struct money_t {
    ullong cents;
};

struct stock_t {
    char *symbol;
    char *fullname;
    struct money_t bought_for;
    ulong count;
    struct money_t price;
};

struct player_t {
    struct money_t cash;
    struct stock_t *portfolio;
    uint portfolio_len;
};
