#include "globals.h"

void buy_handler(struct player_t *player)
{
    printf("Enter the ticker symbol of the stock you wish to purchase: ");
    char *sym = read_ticker();

    struct money_t price;

    printf("Getting stock information...\n");

    char *name;

    if(!get_stock_info(sym, &price, &name))
    {
        printf("Failed to get query information for '%s'.\n", sym);
        free(sym);
        return;
    }

    printf("Stock name: %s\n", name);
    printf("Price per share: $%llu.%02llu\n", price.cents / 100, price.cents % 100);
    printf("Enter the number of shares to be purchased (maximum %llu): ", player->cash.cents / price.cents);

    ullong count = read_int();

    if(count <= 0)
    {
        printf("Purchase cancelled.\n");
        return;
    }

    ullong cost = price.cents * count;

    if(cost > player->cash.cents)
    {
        printf("Not enough money!\n");
        return;
    }

    printf("This will cost $%llu.%02llu. Are you sure? ", cost / 100, cost % 100);
    char *response = read_string();
    all_lower(response);

    if(response[0] == 'y')
    {
        printf("Confirmed.\n");

        struct stock_t *stock = find_stock(player, sym);

        /* add the stock to the portfolio or increase the count of a stock */

        if(stock)
        {
            stock->count += count;
            stock->current_price.cents = price.cents;
        }
        else
        {
            /* stock is not in portfolio yet, add it */

            player->portfolio_len += 1;
            player->portfolio = realloc(player->portfolio, player->portfolio_len * sizeof(struct stock_t));
            player->need_to_free_portfolio = true;

            stock = player->portfolio + player->portfolio_len - 1;

            memset(stock, 0, sizeof(struct stock_t));

            stock->symbol = sym;
            stock->fullname = name;
            stock->count = count;
            stock->current_price.cents = price.cents;
        }

        player->cash.cents -= cost;

        add_hist(stock, BUY, count);

        /* sort the portfolio alphabetically by ticker symbol */
        qsort(player->portfolio, player->portfolio_len, sizeof(struct stock_t), compare_stocks);
    }
    else
    {
        printf("Not confirmed.\n");
    }

    free(response);
}
