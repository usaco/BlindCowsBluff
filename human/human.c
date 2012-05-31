#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <ncurses.h>

#include "bcb-client.h"

char _bot_name[256];
const char* BOT_NAME = _bot_name;

#define xstr(s) _str(s)
#define _str(s) #s

#define PORTNO 1337
#define PORTSTR xstr(PORTNO)

int sockfd;

unsigned int SCREEN_HEIGHT;
unsigned int SCREEN_WIDTH;

#define SCROLLBACK_LINES 512
char scrollback[SCROLLBACK_LINES][80];
unsigned int sb_start = 0u, sb_end = 0u;

#define CARD_HEIGHT 4
#define CARD_WIDTH 6

#define PANEL_HEIGHT 4
#define PANEL_WIDTH(pw) (pw - CARD_WIDTH - 6u)
unsigned gPANELW = 0u;

struct player_vis
{
	WINDOW *card;
	WINDOW *panel;
};

unsigned int starting_money = 0u;
struct player_vis pvis[32];

void error(const char* msg)
{ perror(msg); exit(EXIT_FAILURE); }

WINDOW *player_win;
WINDOW *scroll_win;
WINDOW *action_win;

void update_scrollback();

void draw_card(WINDOW *win, unsigned int card)
{
	werase(win);
	box(win, 0, 0);

	if (card == 0u) mvwaddstr(win, 1, 2, "??");
	else mvwprintw(win, 1, (CARD_WIDTH-1)/2, "%d", card);
	wrefresh(win);
}

void update_panel(WINDOW* win, const struct player_data* pdata, unsigned int pnum)
{
	unsigned int i;
	werase(win);
	
	// coin stack
	int coinw = ((gPANELW-18) - 3) * pdata->pool / starting_money;
	for (i = 0; i < coinw; ++i) mvwprintw(win, 2, i, "(");
	
	// begin bold region
	wattron(win, A_BOLD);
	
	// display bot's name
	if (pnum != SELF.id) mvwprintw(win, 1, 0, "Player %u", pnum + 1);
	else mvwprintw(win, 1, 0, "%s", BOT_NAME);
	
	// display coin graphics
	if (pdata->pool) mvwprintw(win, 2, coinw, "($)");
	
	// end bold region
	wattroff(win, A_BOLD);
	
	// display other info
	mvwprintw(win, 1, gPANELW-15, "POOL:  $%6d", pdata->pool - pdata->wager);
	mvwprintw(win, 2, gPANELW-15, "WAGER: $%6d", pdata->wager);

	wrefresh(win);
}

struct button_t
{
	WINDOW *win;
	char text[32];
	int rtn;
};

#define BTN_COUNT 3
struct button_t buttons[BTN_COUNT];

int was_clicked(const struct button_t* btn, MEVENT* m)
{
	int by, bx, my, mx;
	getbegyx(btn->win, by, bx);
	getmaxyx(btn->win, my, mx);

	if (!(m->bstate & BUTTON1_CLICKED)) return 0;
	return (m->y >= by && m->y <= by + my &&
		m->x >= bx && m->x <= bx + mx);
}

void update_action_panel()
{
	int i, my, mx;
	getmaxyx(action_win, my, mx);

	werase(action_win);
	wborder(action_win, 0, 0, 0, 0, ACS_LTEE, ACS_RTEE, ACS_BTEE, 0);

	mvwprintw(action_win, 1, 2, "POOL:  $%6d", SELF.pool);
	mvwprintw(action_win, 2, 2, "WAGER: $%6d", SELF.wager);
	wrefresh(action_win);

	struct button_t *b;
	for (i = 0, b = buttons; i < BTN_COUNT; ++i, ++b)
	{
		getmaxyx(b->win, my, mx);
		werase(b->win);
		box(b->win, 0, 0);
		mvwprintw(b->win, 1, (mx - strlen(b->text))/2, "%s", b->text);
		wrefresh(b->win);
	}
}

void update_players(const struct player_data* players, unsigned int numplayers)
{
	unsigned int i;
	
	werase(player_win);
	wborder(player_win, 0, 0, 0, 0, 0, ACS_TTEE, 0, ACS_BTEE);
	wrefresh(player_win);

	for (i = 0; i < numplayers; ++i)
	{
		const struct player_data* p = &players[i];
		draw_card(pvis[p->id].card, p->card);
		update_panel(pvis[p->id].panel, p, p->id);
	}
}

void update_screen(const struct player_data* players, unsigned int numplayers)
{
	update_scrollback();
	update_players(players, numplayers);
	update_action_panel();
}

#define sbprintf(...) sprintf(scrollback_buffer(), __VA_ARGS__)

char* scrollback_buffer()
{
	char* bfr = scrollback[sb_end++];
	if (sb_end == sb_start) ++sb_start;
	return bfr;
}

void update_scrollback()
{
	unsigned int i, x, swh, sww;
	getmaxyx(scroll_win, swh, sww);
	sww -= 4u; swh -= 4u;

	werase(scroll_win);
	wborder(scroll_win, 0, 0, 0, 0, ACS_TTEE, 0, ACS_LTEE, ACS_RTEE);
	for (i = sb_start, x = 0; i != sb_end && x < swh; ++i, i%=SCROLLBACK_LINES, x += 2)
		mvwprintw(scroll_win, 2 + x, 2, "%*s", -sww, scrollback[i]);
	wrefresh(scroll_win);
}

#define WAGER_ACTION -100
#define NO_ACTION -101

int process_action(int in)
{
	switch (in)
	{
		case FOLD: return FOLD; break;
		case CALL: return CALL; break;
		default:
			return WAGER(50);
		break;
	}
}

int client_setup(int *argc, char ***argv)
{
	unsigned int i;

	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if (*argc <= 0)
	{
		fprintf(stderr, "No HOSTNAME given\n");
		exit(EXIT_FAILURE);
	}

	int rv; if ((rv = getaddrinfo((*argv)[0], PORTSTR, &hints, &result)))
	{
		fprintf(stderr, "%s\n", gai_strerror(rv));
		error("Could not GETADDRINFO");
	}

	for (rp = result; rp; rp = rp->ai_next)
	{
		sockfd = socket(rp->ai_family, rp->ai_socktype,
			rp->ai_protocol);

		if (sockfd == -1) continue;
		if (connect(sockfd, rp->ai_addr, 
			rp->ai_addrlen) != -1) break;

		close(sockfd);
	}

	if (!rp) error("Could not CONNECT to address");
	freeaddrinfo(result);

	printf("Name: ");
	fgets(_bot_name, sizeof _bot_name, stdin);
	_bot_name[strcspn(_bot_name, "\r\n")] = 0;

	_fdout = sockfd;
	_fdin = sockfd;

	if (NULL == initscr())
	{
		fprintf(stderr, "Could not setup curses window\n");
		exit(EXIT_FAILURE);
	}

	clear(); noecho(); cbreak();
	getmaxyx(stdscr, SCREEN_HEIGHT, SCREEN_WIDTH);

	// left panel width
	unsigned int left_w = SCREEN_WIDTH / 2u;
	if (left_w > SCREEN_WIDTH - 40u) left_w = SCREEN_WIDTH - 40;

	player_win = newwin(SCREEN_HEIGHT, left_w, 0, 0);
	scroll_win = newwin(SCREEN_HEIGHT - 8u, SCREEN_WIDTH - left_w + 1, 0, left_w - 1);
	action_win = newwin(9u, SCREEN_WIDTH - left_w + 1, SCREEN_HEIGHT - 9u, left_w - 1);

	mousemask(BUTTON1_PRESSED, NULL);
	strcpy(buttons[0].text, "CALL");  buttons[0].rtn = CALL;
	strcpy(buttons[1].text, "WAGER"); buttons[1].rtn = WAGER_ACTION;
	strcpy(buttons[2].text, "FOLD");  buttons[2].rtn = FOLD;

	int AH, AW, bw; getmaxyx(action_win, AH, AW);
	AW -= 6u; bw = AW / BTN_COUNT;

	getbegyx(action_win, AH, AW);
	for (i = 0; i < BTN_COUNT; ++i)
		buttons[i].win = newwin(3u, bw, SCREEN_HEIGHT - 4u, AW + 2 + i*(bw + 1));

	sbprintf("Setting up player: %s", _bot_name);
	sbprintf("Mouse is setup? %s", has_mouse() ? "TRUE" : "FALSE");

	update_screen(NULL, 0u);
	return 1;
}

void game_setup(const struct player_data* players, unsigned int numplayers)
{
	unsigned int i, pw, ph;
	sbprintf("Initializing game...");
	getmaxyx(player_win, ph, pw);

	starting_money = 0u;
	for (i = 0; i < numplayers; ++i)
		starting_money += players[i].pool;

	gPANELW = PANEL_WIDTH(pw);
	for (i = 0; i < numplayers; ++i)
	{
		struct player_vis *p = &pvis[i];
		unsigned int cury = 1 + (PANEL_HEIGHT+1) * i;

		p->card = newwin(CARD_HEIGHT, CARD_WIDTH, cury, 2);
		p->panel = newwin(PANEL_HEIGHT, gPANELW, cury, 4+CARD_WIDTH);
	}
	
	update_screen(players, numplayers);
}

unsigned int currentround;
void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante)
{
	currentround = rnum;
	sbprintf("Round #%u ($%u): Player %u begins.", rnum, ante, pstart+1);
	update_scrollback();
}

int player_turn(const struct player_data* players, unsigned int numplayers)
{
	unsigned int i, hwager = 0u;
	const struct player_data* p;

	for (i = 0, p = players; i < numplayers; ++i, ++p)
		if (p->wager > hwager) hwager = p->wager;

	sbprintf("Your turn. Highest wager is currently $%u.", hwager);
	update_screen(players, numplayers);

	for (;;)
	{
		int c = wgetch(action_win);
		MEVENT event;

		//update_scrollback();
		fprintf(stderr, "Got an event! %d (want %d)\n", c, KEY_MOUSE);
		switch (c)
		{
			case KEY_MOUSE:
				if (getmouse(&event) == OK)
				{
					fprintf(stderr, "Clicked @ %d,%d\n", event.y, event.x);
					struct button_t* b; int yy, xx;
					for (i = 0, b = buttons; i < BTN_COUNT; ++i, ++b)
					{
						getbegyx(b->win, yy, xx);
						fprintf(stderr, "Button %s @ %d,%d", b->text, yy, xx);
						if (was_clicked(b, &event))
						{
							sbprintf("Button %s was clicked!", b->text);
							int rtn = process_action(b->rtn);
							if (rtn != NO_ACTION) return rtn;
						}
					}
				}
				return CALL;
				break;
		}
	}
}

void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings)
{
	sbprintf("Round #%u has ended. You won $%u.", currentround, winnings);
	sbprintf("Your card was [%u].", SELF.card);
	update_screen(players, numplayers);
}

void game_end()
{
	endwin();
	close(sockfd);
}

