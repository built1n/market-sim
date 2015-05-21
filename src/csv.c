#include "globals.h"

/* takes a pointer to a pointer to a csv string */
/* the pointer pointed to will be modified, but the data it points to will not */

char *csv_read(char **ptr)
{
    if(!ptr)
        return NULL;

    char *start = *ptr;
    bool quoted = false;

    while(**ptr)
    {
        char c = **ptr;

        if(c == '"')
        {
            quoted = !quoted;
        }

        else if((c == ',' && !quoted)  ||
                c == '\0'              ||
                c == '\n')
        {
            char *ret = malloc(*ptr - start + 1);
            ret[*ptr - start] = '\0';
            memcpy(ret, start, *ptr - start);
            (*ptr)++;

            return ret;
        }

        (*ptr)++;
    }

    return NULL;
}
