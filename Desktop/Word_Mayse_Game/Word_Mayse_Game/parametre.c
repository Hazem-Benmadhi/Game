#include "parametre.h"
#include <stdlib.h>

// {y, x}
const int DIRECTIONS[NUM_DIRECTIONS][2] = {
    {-1, 0}, {1, 0}, {0, -1}, {0, 1}, // Up, Down, Left, Right
    {-1, -1}, {-1, 1}, {1, -1}, {1, 1} // Diagonals
};
int player_steps_num = 0;

int window_width = WINDOW_WIDTH;
int window_height = WINDOW_HEIGHT;


void init_file(File* f) {
    f->header = f->queue = NULL;
}

void enfiler(File* f, position pos, int distance) {
    QueueNode* new_node = (QueueNode*)malloc(sizeof(QueueNode));
    new_node->pos = pos;
    new_node->distance = distance;
    new_node->next = NULL;

    if (f->queue == NULL) {
        f->header = f->queue = new_node;
    }
    else {
        f->queue->next = new_node;
        f->queue = new_node;
    }
}

QueueNode* defiler(File* f) {
    if (f->header == NULL) {
        return NULL;
    }

    QueueNode* temp = f->header;
    f->header = f->header->next;

    if (f->header == NULL) {
        f->queue = NULL;
    }

    return temp;
}

int is_empty(File* f) {
    return f->header == NULL;
}


