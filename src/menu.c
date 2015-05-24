#include "globals.h"

void do_menu(struct player_t *player, const struct command_t *commands, uint len, const char *prompt)
{
    for(uint i = 0; i < len; ++i)
    {
        output("%d. %s\n", i + 1, commands[i].name);
    }

    output("%s", prompt);
    char *cmdbuf = read_string();

    all_lower(cmdbuf);

    /* find the best command */

    int best_command = -1;

    /* first, search for an exact match */
    for(uint i = 0; i < len; ++i)
    {
        if(strcmp(cmdbuf, commands[i].command) == 0)
        {
            best_command = i;
            goto exec_cmd;
        }
    }

    /* look for a number and see if it corresponds to a valid command */
    ulong n = strtol(cmdbuf, NULL, 10);
    if(0 < n && n <= len)
    {
        /* in range, execute */
        best_command = n - 1;
        goto exec_cmd;
    }

    /* now look for a partial match */
    for(uint i = 0; i < len; ++i)
    {
        uint len = strlen(cmdbuf);
        if(len > strlen(commands[i].command))
            continue;
        for(uint j = 1; j <= len; ++j)
        {
            char *buf1 = malloc(j + 1);
            memset(buf1, 0, j + 1);
            memcpy(buf1, cmdbuf, j);
            buf1[j] = '\0';

            char *buf2 = malloc(j + 1);
            memset(buf2, 0, j + 1);
            memcpy(buf2, commands[i].command, j);
            buf2[j] = '\0';

            if(strcmp(buf1, buf2) == 0)
            {
                best_command = i;
                free(buf1);
                free(buf2);
                goto exec_cmd;
            }
            else
            {
                free(buf1);
                free(buf2);
            }
        }
    }

exec_cmd:

    if(best_command >= 0)
    {
        commands[best_command].handler(player);
        output("\n");
    }
    else
    {
        output("Unrecognized command: '%s'\n", cmdbuf);
    }

    free(cmdbuf);
}
