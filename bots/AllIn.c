#include <stdio.h>

#include "bcb-client.h"

const char* BOT_NAME = "AllIn";

int client_setup(int *argc, char ***argv)
{
	return 1;
}

void game_setup(const struct player_data* players, unsigned int numplayers)
{

}

void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante)
{

}

int player_turn(const struct player_data* players, unsigned int numplayers)
{
	return WAGER(SELF.pool);
}

void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings)
{

}

void game_end()
{

}

