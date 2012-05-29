#include <stdio.h>

#include "bcb-client.h"

const char* BOT_NAME = "Conservative";

void game_setup(const struct player_data* players, unsigned int numplayers)
{

}

void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante)
{

}

int player_turn(const struct player_data* players, unsigned int numplayers)
{
	int num_active=0, i, j, cutoff, pending = 0;
	double fraction;

	for(i=0; i<numplayers; i++) {
		num_active+=players[i].active;
		if(players[i].wager>pending)
			pending = players[i].wager;
	}

	fraction = 1.0 - 1.0/((double) num_active);
	fraction = fraction*fraction;

	cutoff = (int)(fraction*((double)XRANGE));

	for(i=0; i<numplayers; i++)
		if(players[i].card>cutoff && players[i].active==1) {
			if(pending>SELF.wager)
				return FOLD;
			else
				return CALL;
		}

	int bet = 2*SELF.wager;
	if(bet>SELF.pool) {
		return WAGER(SELF.pool);
	}

	if(pending>bet && pending<=4*SELF.wager)
		return CALL;
	else if(pending>4*SELF.wager)
		return FOLD;
	else
		return WAGER(bet);
}

void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings)
{

}

void game_end()
{

}

