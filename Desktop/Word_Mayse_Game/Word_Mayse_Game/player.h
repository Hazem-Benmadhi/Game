#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "parametre.h"
#include "maze.h"

typedef struct Player {
    position pos;
    int direction_move;
} Player;

Player* init_player(int x, int y);

int short_path(Grid grid, int rows, int cols, position start, position end, position** path);

int is_valid_word(const char* word, char words[MAX_WORDS][MAX_WORD_LENGTH], int word_count);    // Function to verify if the word exists in the dictionary

#endif
