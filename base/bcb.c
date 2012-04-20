#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <string.h>
#include <time.h>
#include <errno.h>

#define AGENTS 10
#define TIMEOUT_MS 200

#define MSG_BFR_SZ 128

struct agent_t
{
	char name[256];      // bot name
	unsigned int pool;   // current amount of money
	unsigned int wager;  // current wager in this round

// META:
	int status;          // bot's current status
	int fds[2];          // file descriptors to communicate
	int pid;             // process id of agent
};

struct agent_t agents[AGENTS];
static int numagents = AGENTS;

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

void setup_game()
{

};

// engage in a smashing game of Blind Cow's Bluff
void play_game()
{

};

int main(int argc, char** argv)
{
	setbuf(stdout, NULL);
	int i; char msg[MSG_BFR_SZ];
	++argv; --argc;

	numagents = --argc; ++argv;

	// setup all bots provided
	for (i = 0; i < numagents; ++i)
		setup_agent(argv[i], i);

	for (i = 0; i < numagents; ++i)
	{
		listen_bot(msg, i);
		sscanf(msg, "NAME %s", agents[i].name);
	}

	setup_game();
	tell_all("READY", -1);
	play_game();

	cleanup_bots();
};

#define READ 0
#define WRITE 1

#define RUNNING 0
#define ERROR -1

// setup an agent and get its file descriptors
void setup_agent(char* filename, int bot)
{
	int pid, c2p[2], p2c[2];

	// setup anonymous pipes to replace child's stdin/stdout
	if (pipe(c2p) || pipe(p2c))
	{
		// the pipes were not properly set up (perhaps no more file descriptors)
		fprintf(stderr, "Couldn't set up communication for bot%d\n", bot, filename);
		exit(1);
	}

	// fork here!
	switch (pid = fork())
	{
	case -1: // error forking
		agents[bot].status = ERROR;
		fprintf(stderr, "Could not fork process to run bot%d '%s'\n", bot, filename);
		exit(1);

	case 0: // child process
		close(p2c[WRITE]); close(c2p[READ]);
		
		if (STDIN_FILENO != dup2(p2c[READ], STDIN_FILENO))
			fprintf(stderr, "Could not replace stdin on bot%d\n", bot);
		
		if (STDOUT_FILENO != dup2(c2p[WRITE], STDOUT_FILENO))
			fprintf(stderr, "Could not replace stdout on bot%d\n", bot);

		close(p2c[0]); close(c2p[1]);
		agents[bot].status = RUNNING;
		execl(filename, filename, NULL);
		
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
	char *orig_msg = msg;
	
	// read message from file descriptor for a bot
	read(agents[bot].fds[READ], msg, MSG_BFR_SZ);

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
	int i; for (i = 0; i < numagents; ++i)
	if (i != exclude) tell_bot(msg, i);
};

// close all bots' file descriptors
void cleanup_bots()
{
	int i; for (i = 0; i < numagents; ++i)
	{
		close(agents[i].fds[0]);
		close(agents[i].fds[1]);
		kill(agents[i].pid, SIGTERM);
	};
};

