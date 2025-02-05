#pragma once
#ifndef PARAMETRE_H 
#define PARAMETRE_H

#define MAX_WORDS 100
#define MAX_WORD_LENGTH 20

#define HORIZONTAL 1
#define VERTICAL 0

#define MAX_ROW 25
#define MAX_COL 25

#define NUM_DIRECTIONS 8

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 700

#define CELL_SIZE (WINDOW_WIDTH / cols)

#define PROMPT_COUNT 3

extern const int DIRECTIONS[NUM_DIRECTIONS][2]; // Directions for movement: {dy, dx}

extern int player_steps_num;
extern int window_width;
extern int window_height;

typedef struct position {
    int x;
    int y;
} position;

typedef struct QueueNode {
    position pos;
    int distance;
    struct QueueNode* next;
} QueueNode;

typedef struct File {
    QueueNode* header;
    QueueNode* queue;
} File;

void init_file(File* f);

void enfiler(File* f, position pos, int distance);

QueueNode* defiler(File* f);

int is_empty(File* f);

#endif
