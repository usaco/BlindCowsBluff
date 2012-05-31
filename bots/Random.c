#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bcb-client.h"

const char* BOT_NAME = "Random";

int client_setup(int *argc, char ***argv)
{
	return 1;
}

void game_setup(const struct player_data* players, unsigned int numplayers)
{
	srand(time(NULL));
}

void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante)
{

}

int player_turn(const struct player_data* players, unsigned int numplayers)
{
	int i, pending=0;

	for(i=0; i<numplayers; i++) {
		if(players[i].card==0)
			continue;
		if(players[i].wager>pending)
			pending=players[i].wager;
	}

	int need_to_bet = 0;

	if(pending>SELF.wager)
		need_to_bet = 1;

	int bet_value;

	switch(rand()%3) {
		case 0:
		case 1:
			if(need_to_bet) 
				return -1;
			else
				return 0;
			break;
		case 2:
			if(need_to_bet)
				return 0;
			else
				bet_value = SELF.wager+rand()%(SELF.pool-SELF.wager)+1;
			return WAGER(bet_value);
			break;
		default:
			return -1;
			break;
	}
}

void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings)
{

}

void game_end()
{

}

