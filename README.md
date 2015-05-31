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

This will install the market-sim binary in `/usr/local/bin/`.

### Building Without Curses Support

Uncomment the `#define WITHOUT_CURSES` line in `src/globals.h`, then build as described above.

## Basic Tutorial

When you first run `market-sim`, an empty portfolio is created and you are given $1000 USD.

1. You probably want to buy some shares now; type `buy`.

        Enter the ticker symbol of the stock you wish to purchase:

2. Input a ticker symbol.
3. Enter the number of shares you wish to buy.
4. Confirm the transaction. This will return you to the main prompt.
5. From here, you probably want to save your porfolio to disk; type `write`.
6. Enter the name of the file to save to. Careful, there's nothing to keep you from clobbering a file here!
7. Come back later and trade some more!
8. Profit!

## About

Made with boredom and a cheaply bought keyboard.

## Contributions

Contributions are always welcome, see the [TODO list](https://github.com/theunamedguy/market-sim/blob/master/docs/TODO-LIST.md) for some things to do.

## License

Market-Sim is licensed under the [GNU General Public License, version 2](http://www.gnu.org/licenses/gpl-2.0.html) by Franklin Wei.
