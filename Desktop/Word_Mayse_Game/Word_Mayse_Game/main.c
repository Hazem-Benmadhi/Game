#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#include "player.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static TTF_Font* font = NULL;
static SDL_Texture* letter_textures[26];
static int score = 0;
static Uint32 start_time;
static const Uint32 GAME_DURATION = 120000; 
static bool gameEnded = false;
static int gameIsRunning = 0;
static int highest_score = 0;


void render_menu(SDL_Renderer* renderer, TTF_Font* font, int selected_item);
int handle_menu_input(SDL_Event* e, int num_menu_items, int* selected_item);
void render_setting(SDL_Renderer* renderer, TTF_Font* font, int* rows, int* cols, int* difficulty);
int load_highest_score(const char* filename);


void find_random_positions(Grid grid, int rows, int cols, position* start, position* end);
void init_game();
static void render_text(const char* text, int x, int y, SDL_Color color, TTF_Font* font, SDL_Renderer* renderer);
void render_time(Uint32 elapsed_time, TTF_Font* font, SDL_Renderer* renderer, int x, int y);
void render_score(int score, char* string, TTF_Font* font, SDL_Renderer* renderer, int x, int y);
void render_player_word(char* word, TTF_Font* font, SDL_Renderer* renderer, int x, int y);
void remove_word(char* word, char words[MAX_WORDS][MAX_WORD_LENGTH], int* word_count);
void render_maze(Grid grid, Player* player, int rows, int cols, TTF_Font* font, int cell_size, position final, int path_len, position* path);
void render_end(Grid grid, int rows, int cols, position start, position final, int cell_size, TTF_Font* font, int path_len, position* path);

int main(int argc, char* argv[]) {
    srand(time(NULL));

    int rows = 10;
    int cols = 10;
    int difficulty = 1;
    int space = 2;
    char words[MAX_WORDS][MAX_WORD_LENGTH];
    int word_count = 0;
    load_dictionary("dictionary.txt", words, &word_count);
    init_game();
    int selected_item = 0;
    int menu_active = 1;
    SDL_Event event;

    while (menu_active) {
        render_menu(renderer, font, selected_item);
        SDL_RenderPresent(renderer);

        int menu_choice = handle_menu_input(&event, 3, &selected_item);
        if (menu_choice != -1) {
            switch (menu_choice) {
            case 0: // NEW GAME
                gameIsRunning = 1;
                menu_active = 0;
                break;
            case 1: // SETTING
                // Get player input setting
                render_setting(renderer, font, &rows, &cols, &difficulty);
                printf("Rows: %d, Columns: %d, Difficulty: %d\n", rows, cols, difficulty);
                menu_active = 0;
                gameIsRunning = 1;
                break;
            case 2: // EXIT GAME
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                TTF_CloseFont(font);
                SDL_Quit();
                return 0;
            }
        }
    }
    int cell_size = CELL_SIZE;  // (window_width / cols < window_height / rows) ? (window_width / cols) : (window_height / rows);      // Calculate cell size after getting window dimensions
    
    Grid grid;
    init_grid(&grid, rows, cols);
   //// Generate the maze 
    generate_maze(grid, rows, cols, words, word_count, space);
    
    //build_graph(grid, rows, cols, words, word_count);
    position start, final;
    find_random_positions(grid, rows, cols, &start, &final);

    Player* player = init_player(start.x, start.y);
    if (!player) exit(1);    
    start_time = SDL_GetTicks();
    int word_collection_active = 0;
    char collected_word[MAX_WORD_LENGTH] = "";

    position* path = NULL;
    int path_length = 0;
    int path_index = 0;

    while (gameIsRunning) {
        Uint32 elapsed_time = SDL_GetTicks() - start_time;
        if (elapsed_time >= GAME_DURATION || gameEnded) {
            while (true) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        SDL_DestroyRenderer(renderer);
                        SDL_DestroyWindow(window);
                        TTF_CloseFont(font);
                        SDL_Quit();
                        return 0;
                    }
                 }
                path_length = short_path(grid, rows, cols, start, final, &path);
                render_end(grid, rows, cols, start, final, cell_size, font, path_length, path);
                render_score(score, "Game Over! Final Score: %d", font, renderer, cols * cell_size + 30, 150);
                SDL_RenderPresent(renderer);
            }
        }

        SDL_Event event;
        // (1) Handle Input
        // Start Event Loop
        while (SDL_PollEvent(&event)) {
            // Handle each specific event
            if (event.type == SDL_QUIT) {
                gameIsRunning = 0;
            }
            else {
                if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        window_width = event.window.data1;
                        window_height = event.window.data2;
                    }
                }
                cell_size = (window_width / cols < window_height / rows) ? (window_width / cols) : (window_height / rows);
                              
                // Get player direction move, verified then update his position
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_KP_1:
                        player->direction_move = 6; // Bottom-left
                        break;
                    case SDL_SCANCODE_KP_2:
                        player->direction_move = 1; // Down
                        break;
                    case SDL_SCANCODE_KP_3:
                        player->direction_move = 7; // Bottom-right
                        break;
                    case SDL_SCANCODE_KP_4:
                        player->direction_move = 2; // Left
                        break;
                    case SDL_SCANCODE_KP_5:
                        if (word_collection_active == 0) {
                            word_collection_active = 1;
                            collected_word[0] = grid[player->pos.y * cols + player->pos.x]->value;  // Store first letter
                            collected_word[1] = '\0';
                        }
                        else {
                            word_collection_active = 0;
                            if (is_valid_word(collected_word, words, word_count)) {
                                score += strlen(collected_word);    // Add points based on word length
                                remove_word(collected_word, &words, &word_count);    // Remove the word from the dictionary
                                //for (int i = 0; i < word_count; i++) {
                                //    if (strcmp(words[i], collected_word) == 0) {
                                //        // Shift all words down to remove the used word
                                //        for (int j = i; j < word_count - 1; j++) {
                                //            strcpy(words[j], words[j + 1]);
                                //        }
                                //        word_count--;
                                //        break;
                                //    }
                                //}
                            }
                            else {
                                // time (-5) second penality
                                if ((GAME_DURATION - elapsed_time) > 5000) {
                                    start_time -= 5000;
                                }
                                else {
                                    start_time -= GAME_DURATION;
                                }
                            }
                            collected_word[0] = '\0';  // Reset after validation
                        }
                        player->direction_move = -1;
                        break;
                    case SDL_SCANCODE_KP_6:
                        player->direction_move = 3; // Right
                        break;
                    case SDL_SCANCODE_KP_7:
                        player->direction_move = 4; // Top-left
                        break;
                    case SDL_SCANCODE_KP_8:
                        player->direction_move = 0; // Up
                        break;
                    case SDL_SCANCODE_KP_9:
                        player->direction_move = 5; // Top-right
                        break;
                    default:
                        player->direction_move = -1; // Invalid movement
                        break;
                    }
                    if (player->direction_move >= 0 && player->direction_move < NUM_DIRECTIONS) {
                        int dx = player->pos.x + DIRECTIONS[player->direction_move][1]; // New x position
                        int dy = player->pos.y + DIRECTIONS[player->direction_move][0]; // New y position 

                        // Verify the new coordinate
                        if (is_valid(dx, dy, rows, cols) && grid[dy * cols + dx]->value != '#') {
                            player->pos.x = dx;
                            player->pos.y = dy;
                            player_steps_num++;
                            
                            if (word_collection_active) {
                                int len = strlen(collected_word);
                                collected_word[len] = grid[dy * cols + dx]->value;
                                collected_word[len + 1] = '\0';
                            }
                            // Check if player reached the final position
                            if (player->pos.x == final.x && player->pos.y == final.y) {
                                path_length = short_path(grid, rows, cols, start, final, &path);
                                score += ((player_steps_num + 1) == path_length) ? 5 : 0;
                                gameEnded = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        render_maze(grid, player, rows, cols, font, cell_size, final, path_length, path);
        render_time(elapsed_time, font, renderer, cols * cell_size + 30, 50);
        render_score(score, "Score: %d", font, renderer, cols* cell_size + 30, 150);
        render_player_word(collected_word, font, renderer, cols * cell_size + 30, 250);

        SDL_RenderPresent(renderer);    // Finally show what we've drawn
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    SDL_Quit();
    // Free allocated memory
    for (int i = 0; i < rows * cols; i++) {
        free(grid[i]);
    }
    free(grid);
    free(player);
    return 0;
}

void find_random_positions(Grid grid, int rows, int cols, position* start, position* end) {

    do {
        start->x = rand() % cols;
        start->y = rand() % rows;
    } while (grid[start->y * cols + start->x]->value == '#');  // Ensure not a wall

    do {
        end->x = rand() % cols;
        end->y = rand() % rows;
    } while ((grid[end->y * cols + end->x]->value == '#') || 
        (end->x == start->x && end->y == start->y));  // Ensure it's not a wall and different from start
}

void init_game() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow("Maze_word_Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() == -1) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL_ttf: %s", TTF_GetError());
        exit(1);
    }

    font = TTF_OpenFont("fonts/LoveDays.ttf", 18); // Taille de la police : 24 pixels
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font: %s", TTF_GetError());
        exit(1);
    }
    SDL_GetWindowSize(window, &window_width, &window_height);
}

static void render_text(const char* text, int x, int y, SDL_Color color, TTF_Font* font, SDL_Renderer* renderer) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Text rendering failed: %s", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void render_time(Uint32 elapsed_time, TTF_Font* font, SDL_Renderer* renderer, int x, int y) {
    
    SDL_Color color = { 51, 102, 255, 255 };
    Uint32 remaining_time = (GAME_DURATION - elapsed_time) / 1000;

    char time_text[50];
    snprintf(time_text, sizeof(time_text), "Remaining Time: %u", remaining_time);

    SDL_Surface* surface = TTF_RenderText_Solid(font, time_text, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Text rendering failed: %s", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void render_score(int score, char* string, TTF_Font* font, SDL_Renderer* renderer, int x, int y)
{
    SDL_Color color = { 51, 102, 255, 255 };
    char score_text[30];
    snprintf(score_text, sizeof(score_text), string, score);

    SDL_Surface* surface = TTF_RenderText_Solid(font, score_text, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Text rendering failed: %s", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void render_player_word(char* word, TTF_Font* font, SDL_Renderer* renderer, int x, int y) {

    SDL_Color color = { 51, 102, 255, 255 };
    char player_text[50];
    snprintf(player_text, sizeof(player_text), "Player word: %s", word);

    SDL_Surface* surface = TTF_RenderText_Solid(font, player_text, color);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Text rendering failed: %s", TTF_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create texture: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect dest = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void remove_word(char* word, char words[MAX_WORDS][MAX_WORD_LENGTH], int* word_count)
{
    for (int i = 0; i < (*word_count); i++) {
        if (strcmp(words[i], word) == 0) {
            // Shift all words down to remove the used word
            for (int j = i; j < (*word_count) - 1; j++) {
                strcpy_s(words[j], MAX_WORD_LENGTH, words[j + 1]);
            }
            (*word_count)--;
            break;
        }
    }
}

void render_maze(Grid grid, Player* player, int rows, int cols, TTF_Font* font, int cell_size, position final, int path_len, position* path) {
    //const double now = ((double)SDL_GetTicks()) / 1000.0;

    ///* Choose the color for the frame we will draw. The sine wave trick makes it fade between colors smoothly. */
    //const float red = (float)(0.5 + 0.5 * SDL_sin(now));
    //const float green = (float)(0.5 + 0.5 * SDL_sin(now + M_PI * 2 / 3));
    //const float blue = (float)(0.5 + 0.5 * SDL_sin(now + M_PI * 4 / 3));

    ///* Scale color values from [0.0, 1.0] to [0, 255] */
    //Uint8 red_int = (Uint8)(red * 255);
    //Uint8 green_int = (Uint8)(green * 255);
    //Uint8 blue_int = (Uint8)(blue * 255);

    SDL_SetRenderDrawColor(renderer, 255, 255,255,SDL_ALPHA_OPAQUE);

    SDL_RenderClear(renderer);


    SDL_Color text_color = { 0, 0, 0, 255 }; // Text color: black

    // Draw Maze
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            SDL_Rect cell = { x * cell_size, y * cell_size, cell_size, cell_size }; 

            if (grid[y * cols + x]->value == '#') {
                SDL_SetRenderDrawColor(renderer, 51, 102, 255, 255);
                SDL_RenderFillRect(renderer, &cell);

                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            }
            else if (final.x == x && final.y == y) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Final position (green)
                SDL_RenderFillRect(renderer, &cell);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            }

            if (SDL_RenderDrawRect(renderer, &cell) != 0) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't draw a rectangle: %s", SDL_GetError());
                exit(1);
            }

            // Draw the shortest path
            if (gameEnded) {
                for (int i = 0; i < path_len; i++) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for path  
                    SDL_Rect cell = { path[i].x * cell_size, path[i].y * cell_size, cell_size, cell_size };
                    SDL_RenderFillRect(renderer, &cell);
                }
            }

            // Draw player
            if (player->pos.x == x && player->pos.y == y) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for player
                SDL_RenderFillRect(renderer, &cell);
            }

            // Draw letter
            if (grid[y * cols + x]->value >= 'a' && grid[y * cols + x]->value <= 'z') {
                char letter[2] = { grid[y * cols + x]->value, '\0' }; // Convertir le caractère en chaîne
                render_text(letter, x * cell_size + cell_size / 4, y * cell_size + cell_size / 4, text_color, font, renderer);
            }
        }
    }
}

void render_end(Grid grid, int rows, int cols, position start, position final, int cell_size, TTF_Font* font, int path_len, position* path) 
{
    const double now = ((double)SDL_GetTicks()) / 1000.0;

    const float red = (float)(0.5 + 0.5 * SDL_sin(now));
    const float green = (float)(0.5 + 0.5 * SDL_sin(now + M_PI * 2 / 3));
    const float blue = (float)(0.5 + 0.5 * SDL_sin(now + M_PI * 4 / 3));

    Uint8 red_int = (Uint8)(red * 255);
    Uint8 green_int = (Uint8)(green * 255);
    Uint8 blue_int = (Uint8)(blue * 255);

    SDL_SetRenderDrawColor(renderer, red_int, green_int, blue_int, SDL_ALPHA_OPAQUE);

    SDL_RenderClear(renderer);

    SDL_Color text_color = { 0, 0, 0, 255 }; // Couleur du texte : noir

    // Draw Maze
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            SDL_Rect cell = { x * cell_size, y * cell_size, cell_size, cell_size }; 
            if (grid[y * cols + x]->value == '#') {
                SDL_SetRenderDrawColor(renderer, 51, 102, 255, 255);
                SDL_RenderFillRect(renderer, &cell);

                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            }
            SDL_RenderDrawRect(renderer, &cell);
            /*******************************************************************/
            // Draw the shortest path
            for (int i = 0; i < path_len; i++) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for path  
                SDL_Rect s_cell = { path[i].x * cell_size, path[i].y * cell_size, cell_size, cell_size };
                if (start.x == x && start.y == y) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Start position (Red)
                }
                if (final.x == x && final.y == y) {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Final position (green)
                }
                SDL_RenderFillRect(renderer, &s_cell);
            }

            // Draw letter
            if (grid[y * cols + x]->value >= 'a' && grid[y * cols + x]->value <= 'z') {
                char letter[2] = { grid[y * cols + x]->value, '\0' }; // Convertir le caractère en chaîne
                render_text(letter, x * cell_size + cell_size / 4, y * cell_size + cell_size / 4, text_color, font, renderer);
            }
        }
    }
}

/****************************/
void render_menu(SDL_Renderer* renderer, TTF_Font* font, int selected_item)
{
    highest_score = load_highest_score("highest_score.txt");
    SDL_Color normal_color = { 255, 255, 255, 255 };
    SDL_Color selected_color = { 255, 0, 0, 255 };

    const char* menu_items[] = { "NEW GAME", "SETTING", "EXIT GAME" };
    const int num_menu_items = sizeof(menu_items) / sizeof(menu_items[0]);

    for (int i = 0; i < num_menu_items; ++i) {
        SDL_Color color = (i == selected_item) ? selected_color : normal_color;
        render_text(menu_items[i], 100, 200 + i * 40, color, font, renderer);
    }
    render_score(highest_score, "HIGHEST SCORE: %d", font, renderer, 100, 320);
}
int handle_menu_input(SDL_Event* e, int num_menu_items, int* selected_item)
{
    while (SDL_PollEvent(e) != 0) {
        if (e->type == SDL_QUIT) {
            return 1;
        }
        else if (e->type == SDL_KEYDOWN) {
            switch (e->key.keysym.sym) {
            case SDLK_UP:
                *selected_item = (*selected_item - 1 + num_menu_items) % num_menu_items;
                break;
            case SDLK_DOWN:
                *selected_item = (*selected_item + 1) % num_menu_items;
                break;
            case SDLK_RETURN:
                return *selected_item;
            }
        }
    }
    return -1;
}
void render_setting(SDL_Renderer* renderer, TTF_Font* font, int* rows, int* cols, int* difficulty) {
    const char* prompts[PROMPT_COUNT] = {
        "Enter number of rows (1 - 25): ",
        "Enter number of columns (1 - 25): ",
        "Enter difficulty (1: Easy, 2: Medium, 3: Hard): "
    };

    int input_values[PROMPT_COUNT] = { 0 };
    int current_input_index = 0;
    char input_text[100] = { 0 };
    int input_index = 0;
    char error_message[100] = { 0 }; // Store validation error messages

    SDL_StartTextInput();
    SDL_Event event;
    int setting_active = 1;

    while (setting_active) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                setting_active = 0;
                SDL_StopTextInput();
                return;
            }
            else if (event.type == SDL_TEXTINPUT) {
                if (input_index < sizeof(input_text) - 1 && event.text.text[0] >= '0' && event.text.text[0] <= '9') {
                    strcat_s(input_text, sizeof(input_text), event.text.text);
                    input_index++;
                }
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    int value = atoi(input_text);

                    // Validate input
                    if ((current_input_index == 0 && (value <= 0 || value > MAX_ROW)) ||
                        (current_input_index == 1 && (value <= 0 || value > MAX_COL)) ||
                        (current_input_index == 2 && (value < 1 || value > 3))) {

                        sprintf_s(error_message, sizeof(error_message), "Invalid input! Try again.");
                    }
                    else {
                        input_values[current_input_index] = value;
                        current_input_index++;
                        input_text[0] = '\0';
                        input_index = 0;
                        error_message[0] = '\0'; // Clear error message

                        if (current_input_index == PROMPT_COUNT) {
                            setting_active = 0;
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && input_index > 0) {
                    input_text[--input_index] = '\0';
                }
            }
        }

        // Render setting screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i <= current_input_index && i < PROMPT_COUNT; ++i) {
            render_text(prompts[i], 100, 200 + i * 40, (SDL_Color) { 255, 255, 255, 255 }, font, renderer);
            if (i == current_input_index) {
                render_text(input_text, 100 + strlen(prompts[i]) * 12, 200 + i * 40, (SDL_Color) { 255, 255, 255, 255 }, font, renderer);
            }
            else {
                char value_text[100];
                sprintf_s(value_text, sizeof(value_text), "%d", input_values[i]);
                render_text(value_text, 100 + strlen(prompts[i]) * 12, 200 + i * 40, (SDL_Color) { 255, 255, 255, 255 }, font, renderer);
            }
        }

        // Display error message if any
        if (error_message[0] != '\0') {
            render_text(error_message, 100, 400, (SDL_Color) { 255, 0, 0, 255 }, font, renderer);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_StopTextInput();

    // Assign values to settings
    *rows = input_values[0];
    *cols = input_values[1];
    *difficulty = input_values[2];
}

// Function to load the highest score from a file
int load_highest_score(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file %s for reading\n", filename);
        return 0;
    }
    int highest_score = 0;
    fscanf_s(file, "%d", &highest_score);
    fclose(file);
    return highest_score;
}