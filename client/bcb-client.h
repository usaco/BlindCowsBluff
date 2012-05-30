#pragma once

// actions
#define FOLD -1
#define CALL 0
#define WAGER(n) n

// player information
struct player_data
{
	unsigned int id;
	
	// amount of money remaining and wagered
	unsigned int pool, wager;
	
	// player's card (1..), 0 = not known or invalid
	unsigned int card;
	
	// relevant during round
	unsigned int active;
};

// my bot's data
extern struct player_data SELF;

// card information (range and num of duplicates)
// standard playing cards would be XRANGE = 13, XDUP = 4
extern unsigned int XRANGE, XDUP;

// number of rounds between doubling of ante
extern unsigned int ROUNDS_TO_DBL;

