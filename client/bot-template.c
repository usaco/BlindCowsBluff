#include <stdio.h>

#include "bcb-client.h"

// This is the name that the driver will refer to your bot as.
const char* BOT_NAME = "BOTNAME";

// Return whether setup was successful, bot dies if 0.
int client_setup(int *argc, char ***argv)
{
	return 1;
}

// This function is called when the game begins, and provides initial player pools via the players array.
void game_setup(const struct player_data* players, unsigned int numplayers)
{

}

// This function is called when a round begins, provides round num, starting player, and ante.
void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante)
{

}

// When this function is called, your bot should respond with your move.
int player_turn(const struct player_data* players, unsigned int numplayers)
{

}

// This function is called when the round is over.
void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings)
{

}

// This function is called at the end of the game, as a courtesy.
void game_end()
{

}

