#pragma once
#include "bcb-base.h"

typedef unsigned int card_t;

int setup_bcb_vis(int numagents, struct agent_t *agents, int *argc, char ***argv);

int update_bcb_vis(int numagents, struct agent_t *agents, card_t* cards, const int turn, const char *move);

void close_bcb_vis();

