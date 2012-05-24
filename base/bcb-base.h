#pragma once

struct agent_t
{
	char name[256];       // bot name
	unsigned int pool;    // current amount of money
	int wager;            // current wager in this round
	unsigned char act;    // is the bot still active?

// META:
	int status;           // bot's current status
	int fds[2];           // file descriptors to communicate
	int pid;              // process id of agent

// VISUALIZATION:
	void* vis;            // data for visualisation
};

