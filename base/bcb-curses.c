#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <string.h>
#include <ncurses.h>

#include "bcb-base.h"
#include "bcb-visual.h"

#define CARD_HEIGHT 4
#define CARD_WIDTH 6

#define PANEL_HEIGHT 4
#define PANEL_WIDTH 40

struct display_t
{
	WINDOW* notif;
	WINDOW* card;
	WINDOW* panel;
};

unsigned int sleep_time = 500;

int update_card(struct display_t* disp, int card)
{
	werase(disp->card);
	box(disp->card, 0, 0);

	// print some sort of content for the card
	if (card == -1) mvwaddstr(disp->card, 1, 2, "??");
	else mvwprintw(disp->card, 1, (CARD_WIDTH-1)/2, "%d", card);

	wrefresh(disp->card);
}

void update_panel(struct display_t* disp, struct agent_t* agent, const char* move)
{
	werase(disp->panel);
	
	wattron(disp->panel, A_BOLD);
	mvwprintw(disp->panel, 1, 0, "%s", agent->name);
	wattroff(disp->panel, A_BOLD); 
	
	mvwprintw(disp->panel, 1, 25, "POOL:  $%6d", agent->pool - agent->wager);
	mvwprintw(disp->panel, 2, 25, "WAGER: $%6d", agent->wager);
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
	
	for (i = 0; i < numagents; ++i)
	{
		struct display_t *disp = agents[i].vis = malloc(sizeof *disp);
		int cury = 1 + (PANEL_HEIGHT+1) * i;

		disp->notif = newwin(CARD_HEIGHT, 1, cury, 1);
		disp->card = newwin(CARD_HEIGHT, CARD_WIDTH, cury, 3);
		disp->panel = newwin(PANEL_HEIGHT, PANEL_WIDTH, cury, 4+CARD_WIDTH);

		update_card(disp, -1);
		update_panel(disp, &agents[i], NULL);
	}
	
	// get the sleep time, convert from milliseconds to microseconds
	if (*argc) { --*argc; sscanf((*argv++)[0], "%u", &sleep_time); sleep_time *= 1000u; }

	return 1;
};

int update_bcb_vis(int numagents, struct agent_t *agents, card_t* cards, const int turn, const char *move)
{
	int i;
	for (i = 0; i < numagents; ++i)
	{
		struct display_t* vis = agents[i].vis;
		update_card(vis, cards[i]);
		update_panel(vis, &agents[i], i == turn ? move : NULL);
		
		// draw notification region
		wclear(vis->notif); wattron(vis->notif, A_BOLD);
		if (i == turn) mvwvline(vis->notif, 0, 0, '>', PANEL_HEIGHT);
		wattroff(vis->notif, A_BOLD); wrefresh(vis->notif);
	}
	
	usleep(sleep_time);
};

void close_bcb_vis()
{
	endwin();
};

