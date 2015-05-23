#include "globals.h"

void info_handler(struct player_t *player)
{
    char *sym;
    output("Enter the ticker symbol of the stock to get information for: ");
    sym = read_ticker();

    struct stock_t *stock = find_stock(player, sym);

    if(!stock)
    {
        output("Couldn't find '%s' in portfolio.\n", sym);
        free(sym);
        return;
    }

    output("Updating price data...\n");
    if(!get_stock_info(sym, &stock->current_price, &stock->fullname))
    {
        output("Failed to update prices.\n");
        return;
    }

    free(sym);


    heading("Transaction history for '%s'", stock->symbol);

    print_history(stock);
    horiz_line();
    output("Current price: $%llu.%02llu\n", stock->current_price.cents / 100, stock->current_price.cents % 100);
}
