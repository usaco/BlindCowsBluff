#include <unistd.h>

#include "bcb-base.h"
#include "bcb-visual.h"

int setup_bcb_vis(int numagents, struct agent_t *agents, int *argc, char ***argv)
{
	dup2(STDERR_FILENO, STDOUT_FILENO);
	return 1;
};

int update_bcb_vis(int numagents, struct agent_t *agents, card_t* cards, const int turn, const char *move)
{

};

void close_bcb_vis()
{

};

