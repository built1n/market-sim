#include "globals.h"

void buy_handler(struct player_t *player)
{
    char *sym = malloc(16);
    printf("Enter the ticker symbol of the stock you wish to purchase: ");
    scanf("%15s", sym);
    all_upper(sym);

    struct money_t price;

    printf("Getting stock information...\n");

    char *name;

    if(!get_stock_info(sym, &price, &name))
    {
        printf("Failed to get query information for '%s'.\n", sym);
        return;
    }

    printf("Stock name: %s\n", name);
    printf("Price per share: $%llu.%02llu\n", price.cents / 100, price.cents % 100);
    printf("Enter the number of shares to be purchased (maximum %llu): ", player->cash.cents / price.cents);

    ullong count = 0;

    scanf("%llu", &count);

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
    char response[16];
    scanf("%15s", response);
    all_lower(response);

    if(response[0] == 'y')
    {
        printf("Confirmed.\n");

        struct stock_t *stock;

        /* add the stock to the portfolio or increase the count of a stock */

        for(uint i = 0; i < player->portfolio_len; ++i)
        {
            if(strcmp(player->portfolio[i].symbol, sym) == 0)
            {
                stock = player->portfolio + i;
                stock->count += count;
                stock->current_price.cents = price.cents;
                goto done;
            }
        }

        /* not found, add a new one */
        player->portfolio_len += 1;
        player->portfolio = realloc(player->portfolio, player->portfolio_len * sizeof(struct stock_t));
        player->need_to_free_portfolio = true;

        stock = player->portfolio + player->portfolio_len - 1;

        memset(stock, 0, sizeof(struct stock_t));

        stock->symbol = sym;
        stock->fullname = name;
        stock->count = count;
        stock->current_price.cents = price.cents;

    done:

        player->cash.cents -= cost;

        add_hist(stock, BUY, count);

        /* sort the portfolio alphabetically by ticker symbol */
        qsort(player->portfolio, player->portfolio_len, sizeof(struct stock_t), compare_stocks);
    }
    else
    {
        printf("Not confirmed.\n");
    }
}
