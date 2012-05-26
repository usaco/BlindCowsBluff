// FORMAT FOR DRIVER FILE
/*
	<range> <xdup> <ante> <rtd>
	<NUMAGENTS>

	<agentexec>
*/

#define DEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include <string.h>
#include <time.h>

#include <errno.h>
#include <signal.h>

#include "bcb-base.h"
#include "bcb-visual.h"

#define MAXAGENTS 16
#define TIMEOUT_MS 200

#define MSG_BFR_SZ 128

/*#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })*/

#define max(a,b) (a > b ? a : b)
#define min(a,b) (a < b ? a : b)

struct agent_t agents[MAXAGENTS];
static unsigned int NUMAGENTS = MAXAGENTS;

// file pointer to data for setting up the game
FILE* gamedata = NULL;

// range of cards, and number of duplicates
unsigned int xrange = 13;
unsigned int xdup = 4;

// initial ante and rounds-until-double
unsigned int ante = 1;
unsigned short rtd = 10;

// card stuff. all available cards, with the ability to shuffle
card_t *ALL_CARDS;
void shuffle(card_t*, unsigned int);

// sidepots are annoying
void resolve_sidepots(unsigned int*, unsigned int);

// setup a child and get its file descriptors
void setup_agent(char*, int);

// listen to a bot for a message
void listen_bot(char*, int);

// listen to a bot, with a limited amount of time to wait
void listen_bot_timeout(char*, int, int timeout_ms);

// tell a bot a message
void tell_bot(char*, int);

// tell all bots (but one, possibly) a piece of data
void tell_all(char*, int);

// close all bots' file descriptors
void cleanup_bots();

unsigned int starting_money = 0u;
void setup_game(unsigned int NUMAGENTS)
{
	unsigned int i; char msg[MSG_BFR_SZ];

	// setup all bots provided
	for (i = 0; i < NUMAGENTS; ++i)
	{
		// get the pool for this bot
		fscanf(gamedata, "%u", &agents[i].pool);
		starting_money += agents[i].pool;
	
		// get the command for this bot
		fgets(msg, MSG_BFR_SZ, gamedata);

		// remove newline for command
		char *p = strchr(msg, '\n');
		if (p) *p = 0;
		
		// setup the agent using this command
		for (p = msg; *p && isspace(*p); ++p);
		setup_agent(p, i);
	}

	for (i = 0; i < NUMAGENTS; ++i)
	{
		// tell the bot its ID, it should respond with its name
		sprintf(msg, "INIT %u", i); tell_bot(msg, i);
		listen_bot(msg, i);
		
		strncpy(agents[i].name, msg + 5, 255);
		agents[i].name[255] = 0;
	}

	sprintf(msg, "PLAYERS %u", NUMAGENTS);
	tell_all(msg, -1);

	for (i = 0; i < NUMAGENTS; ++i)
	{
		sprintf(msg, "%d %d", i, agents[i].pool);
		tell_all(msg, -1);
	}

	sprintf(msg, "CARDS %u %u", xrange, xdup);
	tell_all(msg, -1);

	// generate the list of cards (this will be shuffled for each round)
	ALL_CARDS = malloc(xrange * xdup * sizeof *ALL_CARDS);
	for (i = 0; i < xrange * xdup; ++i) ALL_CARDS[i] = 1 + i % xrange;

	sprintf(msg, "ANTE %u %u", ante, rtd);
	tell_all(msg, -1);
}

// shuffle them reeeeeeeeeeeeeeeeal good! (also, moderately inefficiently)
void shuffle(card_t *cards, unsigned int num)
{
	unsigned int i = num;
	while (--i)
	{
		int j = rand() % i;
		card_t t = cards[j];
		cards[j] = cards[i]; cards[i] = t;
	}
}

// engage in a smashing game of Blind Cow's Bluff
void play_game()
{
	char msg[MSG_BFR_SZ];
	unsigned int i, pstart, rnum;
	struct agent_t *a;
	
	for (i = 0, a = agents; i < NUMAGENTS; ++i, ++a)
		a->act = a->pool > 0;

	for (pstart = rand() % NUMAGENTS, rnum = 0;; ++rnum)
	{
		unsigned int pturn = pstart;
		unsigned int praise = pstart, turns;

		// if this is a doubling round, double the ante
		if (rnum && rtd && !(rnum % rtd)) ante *= 2u;
		for (i = 0, a = agents; i < NUMAGENTS; ++i, ++a)
		{
			// wager amount cannot exceed your pool size
			unsigned int wager_amt = min(ante, a->pool);

			// intialize wager to ante, and deduct it from pool
			a->wager = !a->act ? 0 : wager_amt;
		}

		// announce all the round information
		sprintf(msg, "ROUND %u %u %u", 1+rnum, pstart, ante);
		tell_all(msg, -1);

		// shuffle the cards, top N cards for players
		shuffle(ALL_CARDS, xrange * xdup);
		update_bcb_vis(NUMAGENTS, agents, ALL_CARDS, pturn, NULL);

		// play rounds
		for (turns = 0; pturn != praise || !turns; ++turns)
		{
			char action[MSG_BFR_SZ], valid = 0;
			unsigned int new_wager;

			// update the current player on the state of his opponents
			tell_bot("TURN", pturn);
			for (i = 0, a = agents; i < NUMAGENTS; ++i, ++a)
			{
				sprintf(msg, "%u %u %u %d %u", i, 
					i == pturn || !a->act ? 0 : ALL_CARDS[i],
					a->pool, a->wager, a->act);
				tell_bot(msg, pturn);
			}

			// let the player make his move (with timeout)
			tell_bot("GO", pturn);
			listen_bot_timeout(msg, pturn, TIMEOUT_MS);

			// get the action off the message
			sscanf(msg, "%s", action);

			// match the highest wager made so far
			if (!strcmp("CALL", action))
			{
				new_wager = agents[praise].wager;
				agents[pturn].wager = min(new_wager, agents[pturn].pool);
				valid = 1;
			}

			// this is a fresh wager (specific value)
			if (!strcmp("WAGER", action))
			{
				sscanf(msg, "%*s %u", &new_wager);
				valid = (new_wager == agents[pturn].pool) /* all-in */
					|| (new_wager >= agents[praise].wager && new_wager <= agents[pturn].pool);
				if (valid)
				{
					agents[pturn].wager = new_wager;
					if (new_wager > agents[praise].wager) praise = pturn;
				};
			}

			// fold is the action we take if we ever receive invalid input
			if (!strcmp("FOLD", action) || !valid) agents[pturn].act = 0;
		
			// update vis
			char tsafe[MSG_BFR_SZ]; strncpy(tsafe, msg, MSG_BFR_SZ);
			update_bcb_vis(NUMAGENTS, agents, ALL_CARDS, pturn, tsafe);
		
			// get next player
			do { ++pturn; pturn %= NUMAGENTS; a = &agents[pturn]; }
			while ( pturn != praise && (!a->act || a->wager >= a->pool) );
		}

		// ugh, sidepots
		unsigned int winnings[MAXAGENTS];
		resolve_sidepots(winnings, NUMAGENTS);
		
		int activeplayers = 0;
		for (i = 0; i < NUMAGENTS; ++i)
		{
			// set whether or not the player is active
			if (agents[i].act = (agents[i].pool > 0u)) ++activeplayers;
		
			// send out information about winnings
			sprintf(msg, "ENDROUND %u", winnings[i]); tell_bot(msg, i);
		}
		
		for (i = 0, a = agents; i < NUMAGENTS; ++i, ++a)
		{
			sprintf(msg, "%u %u %u", i, ALL_CARDS[i], a->pool);
			tell_all(msg, -1);
		}
		
		update_bcb_vis(NUMAGENTS, agents, ALL_CARDS, pturn, NULL);
		if (activeplayers <= 1) break;
		
		// set the next round starter to the next active player
		do { ++pstart; pstart %= NUMAGENTS; }
		while ( !agents[pstart].act || !agents[pstart].pool );
	}
	
	tell_all("ENDGAME", -1);
}

// sidepots are annoying!!!! (add an exclamation mark everytime i say this out loud)
void resolve_sidepots(unsigned int *winnings, unsigned int z)
{
	unsigned int i, minwager, pot, hc, dist;
	struct agent_t *a = NULL;
	
	for (i = 0, a = agents; i < z; ++i, ++a)
	{ a->pool -= a->wager; winnings[i] = 0u; }
	
	for (;;)
	{
		card_t highcard = 0;
		
		// figure out our current wager level
		for (i = 0, a = agents, minwager = UINT_MAX; i < z; ++i, ++a)
			if (a->wager) minwager = min(minwager, a->wager);
			
		// if there are no wagers left to resolve, we are done
		if (minwager == UINT_MAX) break;
		
		// determine the highest card for this sidepot and pot size
		for (i = 0, a = agents, pot = 0u; i < z; ++i, ++a) 
			if (a->wager >= minwager)
			{
				if (a->act && a->wager > 0) highcard = max(highcard, ALL_CARDS[i]);
				pot += minwager;
			}
		
		// determine number of players in this pot with this card
		for (i = 0, a = agents, hc = 0; i < z; ++i, ++a)
			if (a->act && a->wager >= minwager && ALL_CARDS[i] == highcard) ++hc;
		dist = pot / hc;
			
		// process this sidepot
		for (i = 0, a = agents; i < z; ++i, ++a)
			if (a->wager >= minwager)
			{
				// add to my winnings tally and my pool
				if (a->act && ALL_CARDS[i] == highcard)
				{ a->pool += dist; winnings[i] += dist; pot -= dist; }
				
				// adjust the remainder of my wager and continue
				a->act = !!(a->wager -= minwager);
			}
	}
}

void sighandler(int signum)
{
	fprintf(stderr, "!!! Signal %d\n", signum);
	close_bcb_vis();
	cleanup_bots();
	exit(1);
}

int main(int argc, char** argv)
{
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	
	unsigned int i; char msg[MSG_BFR_SZ];
	++argv; --argc;

	signal(SIGPIPE, sighandler);
	signal(SIGTERM, sighandler);
	srand(time(NULL));
	
	gamedata = fopen(argv[0], "r");
	++argv; --argc;

	fscanf(gamedata, "%u %u %u %u", &xrange, &xdup, &ante, &rtd);
	fscanf(gamedata, "%u", &NUMAGENTS);

	setup_game(NUMAGENTS);
	fclose(gamedata);

	setup_bcb_vis(NUMAGENTS, agents, &argc, &argv);

	tell_all("READY", -1);
	play_game();

	close_bcb_vis();
	cleanup_bots();
};

#define READ 0
#define WRITE 1

#define RUNNING 0
#define ERROR -1

// setup an agent and get its file descriptors
void setup_agent(char* filename, int bot)
{
	int i, pid, c2p[2], p2c[2];
	
	char *p, *arglist[100];
	for (i = 0, p = strtok(filename, " "); p; 
		++i, p = strtok(NULL, " ")) arglist[i] = strdup(p);
	arglist[i] = NULL;

	// setup anonymous pipes to replace child's stdin/stdout
	if (pipe(c2p) || pipe(p2c))
	{
		// the pipes were not properly set up (perhaps no more file descriptors)
		fprintf(stderr, "Couldn't set up communication for bot%d\n", bot);
		exit(1);
	}

	// fork here!
	switch (pid = fork())
	{
	case -1: // error forking
		agents[bot].status = ERROR;
		fprintf(stderr, "Could not fork process to run bot%d: '%s'\n", bot, filename);
		exit(1);

	case 0: // child process
		close(p2c[WRITE]); close(c2p[READ]);
		
		if (STDIN_FILENO != dup2(p2c[READ], STDIN_FILENO))
			fprintf(stderr, "Could not replace stdin on bot%d\n", bot);
		
		if (STDOUT_FILENO != dup2(c2p[WRITE], STDOUT_FILENO))
			fprintf(stderr, "Could not replace stdout on bot%d\n", bot);

		close(p2c[0]); close(c2p[1]);
		agents[bot].status = RUNNING;
		execvp(arglist[0], arglist);
		
		agents[bot].status = ERROR;
		fprintf(stderr, "Could not exec bot%d: [%d] %s\n", 
			bot, errno, strerror(errno));

		exit(1);
		break;

	default: // parent process
		agents[bot].pid = pid;
		close(p2c[READ]); close(c2p[WRITE]);

		agents[bot].fds[READ ] = c2p[READ ]; // save the file descriptors in the
		agents[bot].fds[WRITE] = p2c[WRITE]; // returned parameter
		
		return;
	}
}

// listen to a bot for a message
void listen_bot(char* msg, int bot)
{
	// read message from file descriptor for a bot
	memset(msg, 0, MSG_BFR_SZ);
	int br = read(agents[bot].fds[READ], msg, MSG_BFR_SZ);
	
	msg[strcspn(msg, "\r\n")] = 0; // clear out newlines
	if (DEBUG) fprintf(stderr, "--> RECV [%d]: %s\n", bot, msg);
}

// listen to a bot, with a limited amount of time to wait
void listen_bot_timeout(char* msg, int bot, int milliseconds)
{
	// for now...
	listen_bot(msg, bot);
}

// tell a bot a message
void tell_bot(char* msg, int bot)
{
	// write message to file descriptor for a bot
	write(agents[bot].fds[WRITE], msg, strlen(msg));
	write(agents[bot].fds[WRITE], "\n", 1);
	if (DEBUG) fprintf(stderr, "<-- SEND [%d]: %s\n", bot, msg);
}

// tell all bots (but one, possibly) a piece of data
void tell_all(char* msg, int exclude)
{
	int i; for (i = 0; i < NUMAGENTS; ++i)
	if (i != exclude) tell_bot(msg, i);
};

// close all bots' file descriptors
void cleanup_bots()
{
	int i; for (i = 0; i < NUMAGENTS; ++i)
	{
		close(agents[i].fds[0]);
		close(agents[i].fds[1]);
		kill(agents[i].pid, SIGTERM);
	};
};

