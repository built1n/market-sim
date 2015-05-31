Market-Sim
==========

A retro stock-trading game utilizing live data.

Made mostly for fun, there's a very slow, CGI-based demo version available [here](http://fwei.ml/market-sim.html).

## Building

### Prerequisites

 - libcurl
 - libcurses or equivalent (optional, see below)

Just run:

    make ; sudo make install

This will install the market-sim binary in `/usr/local/bin`.

### Building Without Curses Support

Uncomment the `#define WITHOUT_CURSES` line in `src/globals.h`, then build as described above.

## About

Made with boredom and a cheaply bought keyboard.

## Contributions

Contributions are always welcome, see the [TODO list](https://github.com/theunamedguy/market-sim/blob/master/docs/TODO-LIST.md) for some things to do.

## License

Market-Sim is licensed under the [GNU General Public License, version 2](http://www.gnu.org/licenses/gpl-2.0.html).
