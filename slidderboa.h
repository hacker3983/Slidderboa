#ifndef _SNAKEGAME_H
#define _SNAKEGAME_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "slidderboa_assetmanager_types.h"
#include <time.h>

enum slidderboa_snake_direction {
    SLIDDERBOA_SNAKEDIRECTIONUP,
    SLIDDERBOA_SNAKEDIRECTIONDOWN,
    SLIDDERBOA_SNAKEDIRECTIONLEFT,
    SLIDDERBOA_SNAKEDIRECTIONRIGHT
};

typedef struct slidderboa {
    SDL_Event e;
    SDL_Renderer* renderer;
    SDL_Window* window;
    TTF_Font* font;
    int font_size, win_width, win_height;
    SDL_Rect* snake, food, score_canvas[2], score_bar;
    int mouse_x, mouse_y;
    bool mouse_clicked;
    SDL_Texture* food_texture;
    size_t snake_bodysegment_index, snake_bodysegment_count;
    slidderboa_assetmanager_t asset_manager;
    Uint32 snake_deltatime;
    int snake_direction;
    size_t score, highscore;
    bool snake_moving, snake_died, ate_food, generated_food, snake_created, quit;
} slidderboa_t;

void slidderboa_game_create(slidderboa_t* game);
void slidderboa_game_run(slidderboa_t* game);
void slidderboa_game_setwindowcolor(slidderboa_t* game, const SDL_Color color);
void slidderboa_game_getwindowsize(slidderboa_t* game);
int slidderboa_game_getsize_tlength(size_t number);
void slidderboa_game_reset(slidderboa_t* game);
bool slidderboa_game_rect_hover(slidderboa_t* game, SDL_Rect rect);
char* slidderboa_game_sizet_tostring(size_t number, size_t* number_lengthptr);
void slidderboa_game_handle_events(slidderboa_t* game);
void slidderboa_game_handlemovement(slidderboa_t* game);
void slidderboa_game_handlefood_collision(slidderboa_t* game);
void slidderboa_game_handlecollision(slidderboa_t* game);
void slidderboa_game_handlewall_collision(slidderboa_t* game);
void slidderboa_game_createsnake_head(slidderboa_t* game);
void slidderboa_game_generatefood(slidderboa_t* game);
void slidderboa_game_rendersnake(slidderboa_t* game);
void slidderboa_game_displaygame_over(slidderboa_t* game);
void slidderboa_game_renderscore(slidderboa_t* game);
void slidderboa_game_renderpresent(slidderboa_t* game);
void slidderboa_game_initializesnake_bodysegment(slidderboa_t* game, int x, int y, size_t snakebody_segmentindex);
void slidderboa_game_destroy(slidderboa_t* game);
#endif