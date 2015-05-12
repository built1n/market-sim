#include "globals.h"

void sell_handler(struct player_t *player)
{
    char *sym = malloc(16);
    printf("Enter the ticker symbol of the stock you wish to sell: ");
    scanf("%15s", sym);
    all_upper(sym);

    printf("Getting stock information...\n");

    struct stock_t *stock = NULL;
    uint stock_idx;

    for(stock_idx = 0; stock_idx < player->portfolio_len; ++stock_idx)
    {
        if(strcmp(player->portfolio[stock_idx].symbol, sym) == 0)
        {
            stock = player->portfolio + stock_idx;
            break;
        }
    }

    if(!stock)
    {
        printf("Couldn't find '%s' in portfolio.\n", sym);
        return;
    }

    update_handler(player);

    printf("You currently own %llu shares of '%s' (%s) valued at $%llu.%02llu each.\n",
           stock->count, stock->fullname, stock->symbol, stock->current_price.cents / 100, stock->current_price.cents % 100);

    ullong sell = 0;
    printf("How many shares do you wish to sell? ");
    scanf("%llu", &sell);

    if(!sell)
    {
        printf("Sale cancelled.\n");
        return;
    }

    if(stock->count < sell)
    {
        printf("You don't own enough shares!\n");
        return;
    }

    ullong sell_total = stock->current_price.cents * sell;

    printf("This will sell %llu shares for $%llu.%02llu total. Proceed? ", sell, sell_total / 100, sell_total % 100);

    char response[16];
    scanf("%15s", response);
    all_lower(response);

    if(response[0] == 'y')
    {
        stock->count -= sell;

        if(stock->count == 0)
        {
            /* remove this item from the portfolio */
            memmove(player->portfolio + stock_idx, player->portfolio + stock_idx + 1, sizeof(struct stock_t) * (player->portfolio_len - stock_idx - 1));
            player->portfolio_len -= 1;
            player->portfolio = realloc(player->portfolio, sizeof(struct stock_t) * player->portfolio_len);
        }

        player->cash.cents += sell_total;

        printf("%llu shares sold for $%llu.%02llu total.\n", sell, sell_total / 100, sell_total % 100);
    }
    else
    {
        printf("Not confirmed.\n");
    }
}
