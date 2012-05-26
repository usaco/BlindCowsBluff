## What is Blind Man's Bluff?

Each player begins the game with a pool of money. At the beginning of each round, each player will ante-in (make a mandatory fixed wager to participate in the round), and will be given a card that everyone except the player can see. Each player will then take it in turn to bet that they have the highest card, and the player with the highest card wins the contents of the pot. If a player does not have the highest card, they may try bluff other players by wagering higher and higher, encouraging their opponent to fold.

## Getting Started

Begin by cloning a copy of the Git repo:

    git clone git://github.com/authorblues/BlindCowsBluff.git

Go to the base/ directory and compile the driver:

    cd BlindCowsBluff/base
    make

This will create the ncurses Blind Cow's Bluff driver.

[**Optional**] Consider building the example bots to help with testing:

    cd ../client
    make

Make a new bot to begin working. A helper script has been provided for this purpose.

    cd ..
    bash setup-bot.sh "My Awesome Bot"

This will create a new directory bot-MyAwesomeBot/

## Writing a Bot

```c
void game_setup(const struct player_data* players, unsigned int numplayers);
```
This function is called when the game begins, and provides initial player pools via the players array.
  
```c
void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante);
```
This function is called when a round begins, and provides the round number, the player that will begin this round, and the ante (obligatory starting wager) for this round.
  
```c
void player_turn(const struct player_data* players, unsigned int numplayers);
```
When this function is called, your bot should respond with your move. Return either `FOLD`, `CALL`, or `WAGER(n)`. `FOLD` drops out of the round, `CALL` matches the current highest bet (or goes all-in, if you do not have enough money), and `WAGER(n)` updates your wager to "$n"
  
```c
void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings);
```
You will be given information about which card your bot had in the players array. You will also be given the your winnings after sidepots have been computed.
  
```c
void game_end();
```
This function is called at the end of the game, and should be used to clean up any data structures your bot may have used during the game.
  
## More Information

For information on how to run your bot or how to write a bot in a language other than C, check out the BlindCowsBluff wiki: http://github.com/authorblues/BlindCowsBluff/wiki

