BlindCowsBluff
==============

Implementation of Blind Man's Bluff with bots! USACO 2012

## Have questions?

Try the [FAQ/Wiki](http://github.com/authorblues/BlindCowsBluff/wiki).

## Need updates?

If you need to update your local copy, navigate to `BlindCowsBluff/` and try:

    git pull origin master

## What is Blind Man's Bluff?

> The standard version is simply high card wins. Each player is dealt one card that he displays to all other players (traditionally stuck to the forehead facing outwards). This is followed by a round of betting. Players attempt to guess if they have the highest card based on the distribution of visible cards and how other players are betting.

(from [Wikipedia](http://en.wikipedia.org/wiki/Blind_man%27s_bluff_%28poker%29))

## How do I get started?

Begin by cloning a copy of the Git repo:

    git clone git://github.com/authorblues/BlindCowsBluff.git

Go to the `base/` directory and compile the driver:

    cd BlindCowsBluff/base
    make

This will create the [ncurses](http://www.gnu.org/software/ncurses/) Blind Cow's Bluff driver.

[**Optional**] Consider building the example bots to help with testing:

    cd ../client
    make

Make a new bot to begin working. A helper script has been provided for this purpose.

    cd ..
    bash setup-bot.sh "My Awesome Bot"

This will create a new directory `bot-MyAwesomeBot/`