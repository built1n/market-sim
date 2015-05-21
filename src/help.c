#include "globals.h"

void print_usage(int argc, char *argv[])
{
    assert(argc > 1);

    printf("Usage: %s [OPTION] [PORTFOLIO]\n", argv[0]);
    printf("Runs a simulated trading session with PORTFOLIO (creating a new one by default).\n\n");

    printf("Options:\n");
    printf(" -h, --help\tShow this help and exit\n");
    printf(" -v, --verbose\tEnable verbose operation\n");
    printf("     --version\tOutput version information and exit\n");
}

void print_version(void)
{
    printf("market-sim " PROGRAM_VERSION "\n");
    printf("Built with %s.\n", curl_version());
    printf("Build date: %s\n", __DATE__);
    printf("Copyright (C) 2015 Franklin Wei.\n\n");
    printf("License GPLv2: GNU GPL version 2 <http://www.gnu.org/licenses/gpl-2.0.html>\n");
    printf("This program is distributed in the hope that it will be useful, but WITHOUT ANY\n");
    printf("WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A\n");
    printf("PARTICULAR PURPOSE.\n");
    printf("See the GNU General Public License version 2 for more details.\n");
}
