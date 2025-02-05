#pragma once
#ifndef MAZE_H
#define MAZE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parametre.h"

typedef struct Node {
    position p;
    char value;
    struct Node* UP;
    struct Node* DOWN;
    struct Node* LEFT;
    struct Node* RIGHT;
    struct Node* UP_LEFT;
    struct Node* UP_RIGHT;
    struct Node* DOWN_LEFT;
    struct Node* DOWN_RIGHT;
} Node;

typedef Node** Grid;

void init_grid(Grid* grid, int rows, int cols);

void load_dictionary(const char* filename, char words[MAX_WORDS][MAX_WORD_LENGTH], int* word_count);

void generate_maze(Grid grid, int rows, int cols, char words[MAX_WORDS][MAX_WORD_LENGTH], int word_count, int space);

int is_valid(int x, int y, int rows, int cols);

int dfs(Grid grid, int rows, int cols, const char* word, int index, int x, int y, int visited[MAX_ROW][MAX_COL]);

void choose_random_position(int* x, int* y, int rows, int cols, int excluded[MAX_ROW][MAX_COL]);

void reset_temp_directions(int temp_directions[][2], int* size, int directions[][2]);	// Function to reset TEMP_DIRECTIONS to the original DIRECTIONS array

void remove_direction(int temp_directions[][2], int* size, int index);	// Function to remove a direction from TEMP_DIRECTIONS

static void recursive_division(Grid grid, int x1, int y1, int x2, int y2, int cols, int space);

void place_wall(Grid grid, int x1, int y1, int x2, int y2, int passage_x, int passage_y, int cols);


/*******/
void build_graph(Grid grid, int rows, int cols, char* words, int* word_count);
bool canPlaceWord(char* word, int x, int y, int direction, int rows, int cols, Grid grid);
void placeWord(char* word, int x, int y, int cols, int rows, int direction, Grid grid);

#endif
