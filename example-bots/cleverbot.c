#include <stdio.h>

#include "bcb-client.h"

const char* BOT_NAME = "Cleverbot";

void game_setup(const struct player_data* players, unsigned int numplayers)
{
	
}

void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante)
{
	
}

int player_turn(const struct player_data* players, unsigned int numplayers)
{
	unsigned int i, locount = 0, hicount = 0;
	const struct player_data* p;
	
	int dbl = SELF.wager*2 < SELF.pool ? SELF.wager*2 : SELF.pool;
	for (i = 0, p = players; i < numplayers; ++i, ++p)
	{
		if (p->card && p->card <   XRANGE/3) ++locount;
		if (p->card && p->card > 2*XRANGE/3) ++hicount;
	}

	if (locount*2 <= hicount) return FOLD;
	else if (locount >= 2*hicount) return WAGER(dbl);
	else return CALL;
}

void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings)
{
	
}

void game_end()
{

}

