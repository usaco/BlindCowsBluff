#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <curses.h>
#include <form.h>

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

char scrollback[512][80];
char* const *oldscroll = ((char**)scrollback);
char* const *newscroll = ((char**)scrollback);

void error(const char* msg)
{ perror(msg); exit(EXIT_FAILURE); }

WINDOW *player_win;
WINDOW *scroll_win;
WINDOW *action_win;

void process_window(WINDOW *win)
{
	werase(win);
	box(win, 0, 0);
	wrefresh(win);
}

int client_setup(int *argc, char ***argv)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (*argc <= 0)
	{
		fprintf(stderr, "No HOSTNAME given\n");
		exit(EXIT_FAILURE);
	}

	if (getaddrinfo((*argv)[0], PORTSTR, &hints, &result))
		error("Could not GETADDRINFO");

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

	fprintf(stderr, "Socket fd: %d\n", sockfd);
	if (NULL == initscr())
	{
		fprintf(stderr, "Could not setup curses window\n");
		exit(EXIT_FAILURE);
	}

	clear();
	getmaxyx(stdscr, SCREEN_HEIGHT, SCREEN_WIDTH);

	// left panel width
	unsigned int left_w = SCREEN_WIDTH / 2u;
	if (left_w > SCREEN_WIDTH - 40u) left_w = SCREEN_WIDTH - 40;

	player_win = newwin(SCREEN_HEIGHT, left_w, 0, 0);
	scroll_win = newwin(SCREEN_HEIGHT - 8u, SCREEN_WIDTH - left_w + 1, 0, left_w - 1);
	action_win = newwin(9u, SCREEN_WIDTH - left_w + 1, SCREEN_HEIGHT - 9u, left_w - 1);

	process_window(player_win);
	process_window(scroll_win);
	process_window(action_win);
	return 1;
}

void game_setup(const struct player_data* players, unsigned int numplayers)
{

}

void round_start(unsigned int rnum, unsigned int pstart, unsigned int ante)
{

}

int player_turn(const struct player_data* players, unsigned int numplayers)
{
	return CALL;
}

void round_end(const struct player_data* players, unsigned int numplayers, unsigned int winnings)
{

}

void game_end()
{
	endwin();
	close(sockfd);
}

