#include "player.h"
#include <string.h>

Player* init_player(int x, int y) {
    Player* player = (Player*)malloc(sizeof(Player));
    if (!player) {
        fprintf(stderr, "Failed to allocate memory for player\n");
        exit(1); 
    }

    player->pos.x = x;
    player->pos.y = y;
    player->direction_move = 0;

    return player;
}

int short_path(Grid grid, int rows, int cols, position start, position end, position** path)
{
    int visited[MAX_ROW][MAX_COL] = { 0 };
    position parents[MAX_ROW][MAX_COL];
    File f;
    init_file(&f);

    // Enqueue the starting position
    enfiler(&f, start, 0);
    visited[start.y][start.x] = 1;
    parents[start.y][start.x] = (position){ -1, -1 }; // Mark the start with no parent

    while (!is_empty(&f)) {
        QueueNode* current = defiler(&f);
        position pos = current->pos;
        int distance = current->distance;
        free(current);

        // Check if we reached the end position
        if (pos.x == end.x && pos.y == end.y) {
            // Reconstruct the path
            int path_length = distance + 1;
            *path = (position*)malloc(path_length * sizeof(position));
            position cur = end;
            for (int i = path_length - 1; i >= 0; i--) {
                (*path)[i] = cur;
                cur = parents[cur.y][cur.x];
            }
            return path_length;
        }

        // Explore neighbors
        for (int d = 0; d < NUM_DIRECTIONS; d++) {
            int new_x = pos.x + DIRECTIONS[d][1];
            int new_y = pos.y + DIRECTIONS[d][0];

            if (is_valid(new_x, new_y, rows, cols) && !visited[new_y][new_x] && grid[new_y * cols + new_x]->value != '#') {
                visited[new_y][new_x] = 1;
                position new_pos = { new_x, new_y };
                parents[new_y][new_x] = pos; // Track the parent
                enfiler(&f, new_pos, distance + 1);
            }
        }
    }

    return -1; // Path not found
}

int is_valid_word(const char* word, char words[MAX_WORDS][MAX_WORD_LENGTH], int word_count) {
    for (int i = 0; i < word_count; i++) {
        if (strcmp(word, words[i]) == 0) {
            return 1; // Word found
        }
    }
    return 0; // Word not found
}





