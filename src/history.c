#include "globals.h"

void add_hist(struct stock_t *stock, enum history_action action, ullong count)
{
    /* add a history item for this transaction */
    struct history_item *newhist = malloc(sizeof(struct history_item));
    newhist->action = action;
    newhist->count = count;
    newhist->price.cents = stock->current_price.cents;
    newhist->next = NULL;

    time_t timer = time(0);
    struct tm *tm = localtime(&timer);

    newhist->action_time.year = tm->tm_year - (EPOCH_YEAR - 1900);
    newhist->action_time.month = tm->tm_mon;
    newhist->action_time.day = tm->tm_mday;
    newhist->action_time.hour = tm->tm_hour;
    newhist->action_time.minute = tm->tm_min;
    newhist->action_time.second = tm->tm_sec;

    if(!stock->history)
    {
        stock->history = newhist;
    }
    else
    {
        struct history_item *last = stock->history;
        while(last)
        {
            if(last->next)
                last = last->next;
            else
                break;
        }
        last->next = newhist;
    }

    ++stock->history_len;
}

void print_history(struct stock_t *stock)
{
    assert(stock);

    struct history_item *hist = stock->history;

    while(hist)
    {
        ullong total = hist->count * hist->price.cents;

        output("[%d-%d-%d %d:%02d:%02d] ", hist->action_time.year + EPOCH_YEAR, hist->action_time.month + 1, hist->action_time.day + 1,
               hist->action_time.hour, hist->action_time.minute, hist->action_time.second);

        switch(hist->action)
        {
        case BUY:
            output("[BUY]  %llu shares for $%llu.%02llu each (", hist->count, hist->price.cents / 100, hist->price.cents % 100);
            use_color(COL_GREEN);
            output("+$%llu.%02llu", total / 100, total % 100);
            stop_color(COL_GREEN);
            output(").\n");
            break;
        case SELL:
            output("[SELL] %llu shares for $%llu.%02llu each (", hist->count, hist->price.cents / 100, hist->price.cents % 100);
            use_color(COL_RED);
            output("-$%llu.%02llu", total / 100, total % 100);
            stop_color(COL_RED);
            output(").\n");
            break;
        default:
            output("unknown history enum (%d).\n", hist->action);
            break;
        }
        hist = hist->next;
    }
}
