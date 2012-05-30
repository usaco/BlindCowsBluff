#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bcb-client.h"

#define MAXPLAYERS 16
#define MSG_BFR_SZ 128

// helper macro, sorry for the mess...
#define EXPECTED(m, s) { fprintf(stderr, "Expected command %s, received %s.\n", s, m); return 1; }
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

int main(int argc, char **argv)
{
	int i;
	char msg[MSG_BFR_SZ];
	char tag[MSG_BFR_SZ];

	struct player_data *p;

	--argc; ++argv;
	setbuf(stdout, NULL);
	setbuf(stdin , NULL);

	// only for human clients
	if (!client_setup(&argc, &argv)) return;

	scanf("%*s %d", &SELF.id);
	printf("NAME %s\n", BOT_NAME);

	while (scanf("%s", msg))
	{
		if (!strcmp(msg, "READY")) break;
		else if (!strcmp(msg, "PLAYERS"))
		{
			scanf("%u", &numplayers);			
			for (i = 0, p = players; i < numplayers; ++i, ++p)
			{
				scanf("%u %u", &p->id, &p->pool);
				p->wager = p->card = p->active = 0;
			}
		}
		else if (!strcmp(msg, "CARDS"))
			scanf("%u %u", &XRANGE, &XDUP);
		else if (!strcmp(msg, "ANTE"));
		scanf("%*d %u", &ROUNDS_TO_DBL);
	}

	if (!p) { fprintf(stderr, "No INIT players array received!\n"); return 1; }
	copyself(); game_setup(players, numplayers);

	while (scanf("%s", msg))
	{
		if (!strcmp(msg, "ENDGAME")) break;
		else if (!strcmp(msg, "ROUND"))
		{
			unsigned int rnum, pstart, rante;
			scanf("%u %u %u", &rnum, &pstart, &rante);
			copyself(); round_start(rnum, pstart, rante);

			while (scanf("%s", msg))
			{
				if (!strcmp(msg, "TURN"))
				{
					for (i = 0, p = players; i < numplayers; ++i, ++p)
						scanf("%u %u %u %u %u", &p->id, &p->card, &p->pool, 
							&p->wager, &p->active);

					scanf("%s", msg); if (strcmp(msg, "GO")) EXPECTED(msg, "GO");
					copyself();

					int k = player_turn(players, numplayers);
					if (k > SELF.pool) k = SELF.pool;

					// perform the chosen action
					if (k == CALL) printf("CALL\n");
					else if (k > 0) printf("WAGER %d\n", k);
					else printf("FOLD\n");
				}
				else if (!strcmp(msg, "ENDROUND"))
				{
					int winnings; scanf("%u", &winnings);
					for (i = 0, p = players; i < numplayers; ++i, ++p)
					{
						scanf("%u %u %u", &p->id, &p->card, &p->pool);
						p->wager = p->active = 0;
					}
					copyself(); round_end(players, numplayers, winnings);
					break;
				}
				// got an unexpected message...
				else EXPECTED(msg, "TURN/ENDROUND");
			}
		}
		// got an unexpected message...
		else EXPECTED(msg, "ROUND/ENDGAME");
	}

	game_end();
	return 0;
}
