//#include "menu.h"
//
//void render_menu(SDL_Renderer* renderer, TTF_Font* font, int selected_item) 
//{
//    SDL_Color normal_color = { 255, 255, 255, 255 };
//    SDL_Color selected_color = { 255, 0, 0, 255 };
//
//    const char* menu_items[] = { "NEW GAME", "HIGHEST SCORE", "EXIT GAME" };
//    const int num_menu_items = sizeof(menu_items) / sizeof(menu_items[0]);
//
//    for (int i = 0; i < num_menu_items; ++i) {
//        SDL_Color color = (i == selected_item) ? selected_color : normal_color;
//        render_text(menu_items[i], 100, 100 + i * 40, color, font, renderer);
//    }
//}
//
//int handle_menu_input(SDL_Event* e, int num_menu_items, int* selected_item) 
//{
//    while (SDL_PollEvent(e) != 0) {
//        if (e->type == SDL_QUIT) {
//            return 1;
//        }
//        else if (e->type == SDL_KEYDOWN) {
//            switch (e->key.keysym.sym) {
//            case SDLK_UP:
//                *selected_item = (*selected_item - 1 + num_menu_items) % num_menu_items;
//                break;
//            case SDLK_DOWN:
//                *selected_item = (*selected_item + 1) % num_menu_items;
//                break;
//            case SDLK_RETURN:
//                return *selected_item;
//            }
//        }
//    }
//    return -1;
//}