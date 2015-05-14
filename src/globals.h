#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

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

enum history_action { BUY, SELL };

struct history_item {
    enum history_action action;
    ullong count;
    struct money_t price;

    struct history_item *next;
};

struct stock_t {
    char *symbol;
    char *fullname;
    ullong count;
    struct money_t current_price;
    struct history_item *history;
};

struct player_t {
    struct money_t cash;
    uint portfolio_len;
    struct stock_t *portfolio;
    bool need_to_free_portfolio;
};

/*** prototypes ***/
void cleanup(void);
int compare_stocks(const void*, const void*);
void all_lower(char*);
void all_upper(char*);
bool get_stock_info(char *sym, struct money_t*, char **name);
uint64_t to_sys64(uint64_t);
uint64_t to_be64(uint64_t);
struct stock_t *find_stock(struct player_t*, char*);
void add_hist(struct stock_t*, enum history_action, ullong count);

void buy_handler(struct player_t*);
void sell_handler(struct player_t*);
void history_handler(struct player_t*);
void update_handler(struct player_t*);
void save_handler(struct player_t*);
void load_handler(struct player_t*);
void quit_handler(struct player_t*);
