#include "globals.h"

void info_handler(struct player_t *player)
{
    char *sym;
    printf("Enter the ticker symbol of the stock to get information for: ");
    sym = read_ticker();

    struct stock_t *stock = find_stock(player, sym);

    if(!stock)
    {
        printf("Couldn't find '%s' in portfolio.\n", sym);
        free(sym);
        return;
    }

    free(sym);

    printf("Transaction history:\n");
    print_history(stock);
}