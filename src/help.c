#include "globals.h"

void print_usage(int argc, char *argv[])
{
    assert(argc > 1);

    output("Usage: %s [OPTION] [PORTFOLIO]\n", argv[0]);
    output("Runs an interactive trading session with PORTFOLIO or create a new portfolio.\n\n");

    output("Options:\n");
    output("     --batch\tEnables batch operation, taking commands from standard input\n");
    output(" -h, --help\tShow this help and exit\n");
    output("     --nocurses\tOperate without curses\n");
    output(" -r, --restrict\tOperate in restricted mode, suitable for use as a CGI program\n");
    output(" -v, --verbose\tEnable verbose operation\n");
    output("     --version\tOutput version information and exit\n");
}

void print_version(void)
{
    output("market-sim " PROGRAM_VERSION "\n");
    output("Built with %s.\n", curl_version());
    output("Build date: %s\n", __DATE__);
    output("Copyright (C) 2015 Franklin Wei.\n\n");
    output("License GPLv2: GNU GPL version 2 <http://www.gnu.org/licenses/gpl-2.0.html>\n");
    output("This program is distributed in the hope that it will be useful, but WITHOUT ANY\n");
    output("WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A\n");
    output("PARTICULAR PURPOSE.\n");
    output("See the GNU General Public License version 2 for more details.\n");
}
