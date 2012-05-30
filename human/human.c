#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "bcb-client.h"

char _bot_name[256];
const char* BOT_NAME = _bot_name;

#define xstr(s) _str(s)
#define _str(s) #s

#define PORTNO 1337
#define PORTSTR xstr(PORTNO)

int sockfd;

void error(const char* msg)
{ perror(msg); exit(EXIT_FAILURE); }

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

	if (STDIN_FILENO != dup2(sockfd, STDIN_FILENO) 
		|| STDOUT_FILENO != dup2(sockfd, STDOUT_FILENO))
	{
		fprintf(stderr, "Could not redirect STDIN/STDOUT\n");
		exit(EXIT_FAILURE);
	}
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
	close(sockfd);
}

