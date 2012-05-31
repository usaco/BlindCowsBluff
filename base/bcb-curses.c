#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <string.h>
#include <ncurses.h>

#include "bcb-base.h"
#include "bcb-visual.h"

unsigned int SCREEN_WIDTH = 50u;

#define CARD_HEIGHT 4
#define CARD_WIDTH 6

#define PANEL_HEIGHT 4
#define PANEL_WIDTH (SCREEN_WIDTH - (5 + CARD_WIDTH))

struct display_t
{
	WINDOW* notif;
	WINDOW* card;
	WINDOW* panel;
};

unsigned int sleep_time = 500;

void update_card(struct display_t* disp, int card, int qhigh)
{
	werase(disp->card);
	box(disp->card, 0, 0);
	if (qhigh) wattron(disp->card, A_BOLD);

	// print some sort of content for the card
	if (card == -1) mvwaddstr(disp->card, 1, 2, "??");
	else mvwprintw(disp->card, 1, (CARD_WIDTH-1)/2, "%d", card);

	if (qhigh) wattroff(disp->card, A_BOLD);
	wrefresh(disp->card);
}

void update_panel(struct display_t* disp, struct agent_t* agent, const char* move)
{
	int i;
	werase(disp->panel);
	
	// coin stack
	int coinw = ((PANEL_WIDTH-16) - 3) * agent->pool / starting_money;
	for (i = 0; i < coinw; ++i) mvwprintw(disp->panel, 2, i, "(");
	
	// begin bold region
	wattron(disp->panel, A_BOLD);
	
	// display bot's name
	mvwprintw(disp->panel, 1, 0, "%s", agent->name);
	
	// display coin graphics
	if (agent->pool) mvwprintw(disp->panel, 2, coinw, "($)");
	
	// end bold region
	wattroff(disp->panel, A_BOLD);
	
	// display other info
	mvwprintw(disp->panel, 1, PANEL_WIDTH-15, "POOL:  $%6d", agent->pool - agent->wager);
	mvwprintw(disp->panel, 2, PANEL_WIDTH-15, "WAGER: $%6d", agent->wager);
	if (move) mvwprintw(disp->panel, 1, strlen(agent->name)+1, "{ \"%s\" ) ", move);

	wrefresh(disp->panel);
}

int setup_bcb_vis(int numagents, struct agent_t *agents, int *argc, char ***argv)
{
	int i;
	if (NULL == initscr())
	{
		fprintf(stderr, "initscr() == NULL");
		return 0;
	}

	curs_set(0);
	clear();
	noecho();
	cbreak();
	
	// don't care about y
	getmaxyx(stdscr, i, SCREEN_WIDTH);
	
	for (i = 0; i < numagents; ++i)
	{
		struct display_t *disp = agents[i].vis = malloc(sizeof *disp);
		int cury = 1 + (PANEL_HEIGHT+1) * i;

		disp->notif = newwin(CARD_HEIGHT, 1, cury, 1);
		disp->card = newwin(CARD_HEIGHT, CARD_WIDTH, cury, 3);
		disp->panel = newwin(PANEL_HEIGHT, PANEL_WIDTH, cury, 4+CARD_WIDTH);

		update_card(disp, -1, 0);
		update_panel(disp, &agents[i], NULL);
	}
	
	// get the sleep time, convert from milliseconds to microseconds
	if (*argc) { --*argc; sscanf((*argv++)[0], "%u", &sleep_time); sleep_time *= 1000u; }

	return 1;
};

int update_bcb_vis(int numagents, struct agent_t *agents, card_t* cards, const int turn, const char *move)
{
	int i; card_t highcard = 0u;
	for (i = 0; i < numagents; ++i)
		if (cards[i] > highcard) highcard = cards[i];
	
	struct agent_t *a;
	for (i = 0, a = agents; i < numagents; ++i, ++a)
	{
		struct display_t* vis = a->vis;
		update_card(vis, a->pool ? cards[i] : -1, 
			cards[i] == highcard && a->pool);
		update_panel(vis, &agents[i], i == turn ? move : NULL);
		
		// draw notification region
		wclear(vis->notif); wattron(vis->notif, A_BOLD);
		if (i == turn) mvwvline(vis->notif, 0, 0, '>', PANEL_HEIGHT);
		wattroff(vis->notif, A_BOLD); wrefresh(vis->notif);
	}
	
	usleep(sleep_time);
	return 1;
};

void close_bcb_vis()
{
	endwin();
};


