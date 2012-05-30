#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bcb-client.h"

#define MAXPLAYERS 16
#define MSG_BFR_SZ 128

// helper macro, sorry for the mess...
#define EXPECTED(m, s) { fprintf(stderr, "Expected command %s, received %s.\n", s, m); return EXIT_FAILURE; }
#define copyself() memcpy(&SELF, &players[SELF.id], sizeof SELF)

/* these functions should be defined by the bot author */

extern const char* BOT_NAME;

extern int client_setup(int* /*argc*/, char*** /*argv*/);

extern void game_setup(const struct player_data* /*players*/, 
	unsigned int /*player count*/);

extern void round_start(unsigned int /*round number*/, 
	unsigned int /*starting player*/, unsigned int /*ante*/);

extern int player_turn(const struct player_data* /*players*/, 
	unsigned int /*player count*/);

extern void round_end(const struct player_data* /*players*/, 
	unsigned int /*player count*/, unsigned int /*winnings*/);

extern void game_end();

// ########################################################

unsigned int numplayers = 0;

struct player_data players[MAXPLAYERS];

struct player_data SELF;

unsigned int XRANGE, XDUP;

unsigned int ROUNDS_TO_DBL;

int _fdout = STDOUT_FILENO, _fdin = STDIN_FILENO;

int recv(char* msg)
{
	// read message from file descriptor for a bot
	bzero(msg, MSG_BFR_SZ);
	return read(_fdin, msg, MSG_BFR_SZ);
}

void send(char* msg)
{
	// write message to file descriptor for a bot
	write(_fdout, msg, MSG_BFR_SZ);
}

int main(int argc, char **argv)
{
	int i;
	char msg[MSG_BFR_SZ];
	char tag[MSG_BFR_SZ];

	struct player_data *p;

	--argc; ++argv;
	setbuf(stdout, NULL);
	setbuf(stdin , NULL);

	if (!client_setup(&argc, &argv))
		return EXIT_FAILURE;

	recv(msg); sscanf(msg, "%*s %d", &SELF.id);
	sprintf(msg, "NAME %s", BOT_NAME); send(msg);

	while (recv(msg))
	{
		sscanf(msg, "%s", tag);
		
		if (!strcmp(tag, "READY")) break;
		else if (!strcmp(tag, "PLAYERS"))
		{
			sscanf(msg, "%*s %u", &numplayers);
			for (i = 0, p = players; i < numplayers; ++i, ++p)
			{
				recv(msg); sscanf(msg, "%u %u", &p->id, &p->pool);
				p->wager = p->card = p->active = 0;
			}
		}
		else if (!strcmp(tag, "CARDS"))
			sscanf(msg, "%*s %u %u", &XRANGE, &XDUP);
		else if (!strcmp(tag, "ANTE"));
			sscanf(msg, "%*s %*d %u", &ROUNDS_TO_DBL);
	}

	if (!p) { fprintf(stderr, "No INIT players array received!\n"); return 1; }
	copyself(); game_setup(players, numplayers);

	while (recv(msg))
	{
		sscanf(msg, "%s", tag);
		
		if (!strcmp(tag, "ENDGAME")) break;
		else if (!strcmp(tag, "ROUND"))
		{
			unsigned int rnum, pstart, rante;
			sscanf(msg, "%*s %u %u %u", &rnum, &pstart, &rante);
			copyself(); round_start(rnum, pstart, rante);

			while (recv(msg))
			{
				sscanf(msg, "%s", tag);
				if (!strcmp(tag, "TURN"))
				{
					for (i = 0, p = players; i < numplayers; ++i, ++p)
					{
						recv(msg);
						sscanf(msg, "%u %u %u %u %u", &p->id, &p->card, &p->pool, 
							&p->wager, &p->active);
					}

					copyself(); recv(tag);
					if (strcmp(tag, "GO")) EXPECTED(tag, "GO");

					int k = player_turn(players, numplayers);
					if (k > SELF.pool) k = SELF.pool;

					// perform the chosen action
					switch (k)
					{
						case CALL: sprintf(msg, "CALL"); break;
						case FOLD: sprintf(msg, "FOLD"); break;
						default:   sprintf(msg, "WAGER %d", k); break;
					}
					send(msg);
				}
				else if (!strcmp(tag, "ENDROUND"))
				{
					int winnings; scanf(msg, "%*s %u", &winnings);
					for (i = 0, p = players; i < numplayers; ++i, ++p)
					{
						recv(msg); p->wager = p->active = 0;
						scanf(msg, "%u %u %u", &p->id, &p->card, &p->pool);
					}
					copyself(); round_end(players, numplayers, winnings);
					break;
				}
				// got an unexpected message...
				else EXPECTED(tag, "TURN/ENDROUND");
			}
		}
		// got an unexpected message...
		else EXPECTED(tag, "ROUND/ENDGAME");
	}

	game_end();
	return EXIT_SUCCESS;
}
