#include "maze.h"

#include <string.h>
#include <time.h>

void init_grid(Grid* grid, int rows, int cols) 
{
    int grid_size = rows * cols;
    *grid = (Grid)malloc(grid_size * sizeof(Node*));
    if (*grid == NULL) {
        perror("Failed to allocate grid memory");
        exit(EXIT_FAILURE);
    }

    // Allocate and initialize each node
    for (int i = 0; i < grid_size; i++) {
        (*grid)[i] = (Node*)malloc(sizeof(Node));
        if ((*grid)[i] == NULL) {
            perror("Failed to allocate node memory");
            exit(EXIT_FAILURE);
        }

        (*grid)[i]->p.x = i % cols;
        (*grid)[i]->p.y = i / cols;
        (*grid)[i]->value = ' ';
        (*grid)[i]->UP = (*grid)[i]->DOWN = (*grid)[i]->LEFT = (*grid)[i]->RIGHT = NULL;
        (*grid)[i]->UP_LEFT = (*grid)[i]->UP_RIGHT = (*grid)[i]->DOWN_LEFT = (*grid)[i]->DOWN_RIGHT = NULL;
    }

    // Set neighbors for each node
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            Node* current = (*grid)[y * cols + x];

            if (y > 0) current->UP = (*grid)[(y - 1) * cols + x];
            if (y < rows - 1) current->DOWN = (*grid)[(y + 1) * cols + x];
            if (x > 0) current->LEFT = (*grid)[y * cols + (x - 1)];
            if (x < cols - 1) current->RIGHT = (*grid)[y * cols + (x + 1)];

            if (y > 0 && x > 0) current->UP_LEFT = (*grid)[(y - 1) * cols + (x - 1)];
            if (y > 0 && x < cols - 1) current->UP_RIGHT = (*grid)[(y - 1) * cols + (x + 1)];
            if (y < rows - 1 && x > 0) current->DOWN_LEFT = (*grid)[(y + 1) * cols + (x - 1)];
            if (y < rows - 1 && x < cols - 1) current->DOWN_RIGHT = (*grid)[(y + 1) * cols + (x + 1)];
        }
    }
}

void load_dictionary(const char* filename, char words[MAX_WORDS][MAX_WORD_LENGTH], int* word_count)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open dictionary file");
        exit(EXIT_FAILURE);
    }

    while (fscanf_s(file, "%s", words[*word_count], _countof(words[*word_count])) != EOF) {
        (*word_count)++;
        if (*word_count >= MAX_WORDS) {
            break;
        }
    }

    fclose(file);
}

void generate_maze(Grid grid, int rows, int cols, char words[MAX_WORDS][MAX_WORD_LENGTH], int word_count, int space) 
{
    // Initialize the grid 
    //for (int y = 0; y < rows; y++) {
    //    for (int x = 0; x < cols; x++) {
    //        grid[y * cols + x]->value = ' ';
    //    }
    //}
    //recursive_division(grid, 0, 0, rows - 1, cols - 1, cols, space);

    //// Place words in the maze along paths
    //for (int w = 0; w < word_count; w++) {
    //    int word_length = strlen(words[w]);
    //    printf("%s\n", words[w]);

    //    int visited[MAX_ROW][MAX_COL] = { 0 };
    //    int excluded[MAX_ROW][MAX_COL] = { 0 }; // Tracks excluded positions
    //    int attempts = 0, max_attempts = rows * cols;

    //    int start_x, start_y;
    //    while (attempts < max_attempts) {
    //        choose_random_position(&start_x, &start_y, rows, cols, excluded);
    //        printf("(%d,%d)\n", start_x, start_y);

    //        // Attempt to find a valid path using DFS
    //        if (dfs(grid, rows, cols, words[w], 0, start_x, start_y, visited)) {
    //            printf("The word \"%s\" was successfully placed in the matrix.\n", words[w]);
    //            break;
    //        }

    //        excluded[start_y][start_x] = 1;
    //        attempts++;
    //    }
    //}

    //// Fill remaining cells randomly
    //for (int y = 0; y < rows; y++) {
    //    for (int x = 0; x < cols; x++) {
    //        if (grid[y * cols + x]->value == ' ') {
    //            //if (rand() % 2 == 0) {
    //            //    grid[y * cols + x]->value = '_'; // 50% chance for a '_'
    //            //}
    //            //else {
    //            //    grid[y * cols + x]->value = 'a' + (rand() % 26); // Random letter a-z
    //            //}
    //            grid[y * cols + x]->value = 'a' + (rand() % 26); // Random letter a-z

    //        }
    //    }
    //}
    // Initialize the grid with walls
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            grid[y * cols + x]->value = ' '; // Empty cells in the grid initially
        }
    }

    // Place words in the maze along paths
    for (int w = 0; w < word_count; w++) {
        int word_length = strlen(words[w]);
        printf("%s\n", words[w]);

        int visited[MAX_ROW][MAX_COL] = { 0 };
        int excluded[MAX_ROW][MAX_COL] = { 0 }; // Tracks excluded positions
        int attempts = 0, max_attempts = rows * cols;

        int start_x, start_y;
        while (attempts < max_attempts) {
            choose_random_position(&start_x, &start_y, rows, cols, excluded);
            printf("(%d,%d)\n", start_x, start_y);

            // Attempt to find a valid path using DFS
            if (dfs(grid, rows, cols, words[w], 0, start_x, start_y, visited)) {
                printf("The word \"%s\" was successfully placed in the matrix.\n", words[w]);
                break;
            }

            excluded[start_y][start_x] = 1;
            attempts++;
        }
    }

    // Fill remaining cells randomly
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (grid[y * cols + x]->value == ' ') {
                if (rand() % 2 == 0) {
                    grid[y * cols + x]->value = '#'; // 50% chance for a '#'
                }
                else {
                    grid[y * cols + x]->value = 'a' + (rand() % 26); // Random letter a-z
                }
            }
        }
    }
}

int is_valid(int x, int y, int rows, int cols) 
{
    return x >= 0 && x < cols && y >= 0 && y < rows;
}

void choose_random_position(int* x, int* y, int rows, int cols, int excluded[MAX_ROW][MAX_COL]) 
{
    do {
        *x = rand() % cols;
        *y = rand() % rows;
    } while (excluded[*y][*x]); // Ensure the position is not excluded
}

int dfs(Grid grid, int rows, int cols, const char* word, int index, int x, int y, int visited[MAX_ROW][MAX_COL]) 
{
    // Check if the entire word has been found
    if (word[index] == '\0') {
        return 1;
    }

    // Check for out of bounds, already visited, or mismatch
    if (!is_valid(x, y, rows, cols) || visited[y][x] || grid[y * cols + x]->value != ' ') {
        return 0;
    }

    // Mark current cell as visited
    visited[y][x] = 1;

    int TEMP_DIRECTIONS[NUM_DIRECTIONS][2];
    int temp_size = NUM_DIRECTIONS;

    reset_temp_directions(TEMP_DIRECTIONS, &temp_size, DIRECTIONS);  // Creat and Reset the temporary direction

    // Explore all 8 directions randomly
    while (temp_size > 0) {
        int direction = rand() % temp_size;
        int new_x = x + TEMP_DIRECTIONS[direction][1];
        int new_y = y + TEMP_DIRECTIONS[direction][0];

        remove_direction(TEMP_DIRECTIONS, &temp_size, direction);   // Remove the chosen direction

        if (dfs(grid, rows, cols, word, index + 1, new_x, new_y, visited)) {
            grid[y * cols + x]->value = word[index];
            return 1;
        }
    }

    // Backtrack: Unmark the current position
    visited[y][x] = 0;
    return 0;
}

void reset_temp_directions(int temp_directions[][2], int* size, int directions[][2])
{
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        temp_directions[i][0] = directions[i][0];
        temp_directions[i][1] = directions[i][1];
    }
    *size = NUM_DIRECTIONS; // Reset the size to the original number of directions
}

void remove_direction(int temp_directions[][2], int* size, int index) 
{
    // Shift elements to the left to remove the direction at 'index'
    for (int i = index; i < (*size) - 1; i++) {
        temp_directions[i][0] = temp_directions[i + 1][0];
        temp_directions[i][1] = temp_directions[i + 1][1];
    }
    (*size)--; // Reduce the size of the array
}

static void recursive_division(Grid grid, int x1, int y1, int x2, int y2, int cols, int space) {
    int width = x2 - x1;
    int height = y2 - y1;

    if (width < space || height < space) return;

    int horizontal = rand() % 2;
    if (width > height) horizontal = 0;
    if (height > width) horizontal = 1;

    if (horizontal) {
        int y_cut = y1 + 1 + rand() % (height - 1);
        int passage_x = x1 + rand() % width;


        place_wall(grid, x1, y_cut, x2, y_cut, passage_x, y_cut, cols);
        recursive_division(grid, x1, y1, x2, y_cut - 1, cols, space);
        recursive_division(grid, x1, y_cut + 1, x2, y2, cols, space);
    }
    else {
        int x_cut = x1 + 1 + rand() % (width - 1);
        int passage_y = y1 + rand() % height;
        place_wall(grid, x_cut, y1, x_cut, y2, x_cut, passage_y, cols);

        recursive_division(grid, x1, y1, x_cut - 1, y2, cols, space);
        recursive_division(grid, x_cut + 1, y1, x2, y2, cols, space);
    }
}

void place_wall(Grid grid, int x1, int y1, int x2, int y2, int passage_x, int passage_y, int cols) {
    if (x1 == x2) {
        for (int y = y1; y <= y2; y++) {
            if (y != passage_y && grid[x1 + y * cols]->value == ' ') {
                grid[x1 + y * cols]->value = '#';
            }
        }
    }
    else if (y1 == y2) {
        for (int x = x1; x <= x2; x++) {
            if (x != passage_x && grid[x + y1 * cols]->value == ' ') {
                grid[x + y1 * cols]->value = '#';
            }
        }
    }
}


void build_graph(Grid grid, int rows, int cols, char* words, int word_count) {
    int letter_num = 0;
    while (letter_num < rows * cols * 0.5) {
        int w = rand() % word_count;
        bool placed = false;
        for (int attempt = 0; attempt < 20 && !placed; attempt++)
        {   
            int start_x = rand() % rows;
            int start_y = rand() % cols;
            int direction = rand() % NUM_DIRECTIONS;

            if (canPlaceWord(words[w], start_x, start_y, direction, rows, cols, grid)) {
                placeWord(words[w], start_x, start_y, direction, cols, rows, grid);
                letter_num += strlen(words[w]);
                placed = true;

            }
        }

    }

}

bool canPlaceWord(char* word, int x, int y, int direction, int rows, int cols, Grid grid) {
    
    int dx = DIRECTIONS[direction][0];
    int dy = DIRECTIONS[direction][1];
    int word_length = strlen(word);
    for (int c = 0; c < word_length; c++)
    {
        if (!is_valid(x, y, rows, cols)) return false;
        if (grid[y * cols + x]->value != '#' && grid[y * cols + x]->value != word[c]) return false;
        x += dx;
        y += dy;
    }   
    
    return true;
}

void placeWord(char* word, int x, int y, int cols, int rows, int direction, Grid grid) {
    int dx = DIRECTIONS[direction][0];
    int dy = DIRECTIONS[direction][1];

    for (int c = 0; c < strlen(word); c++) {
        grid[y * cols + x]->value = word[c];
        x += dx;
        y += dy;
    }
}