#include "slidderboa.h"
#include "slidderboa_assetmanager.h"

const SDL_Color WINDOW_COLOR = {0x29, 0x33, 0x5C, 0xFF};
const SDL_Color GAMEOVER_BGCOLOR = {0x12, 0x12, 0x12, 0xFF};
const SDL_Color SNAKE_COLOR = {0xF3, 0xA7, 0x12, 0xFF};
const SDL_Color SCORETEXT_COLOR = {0xE4, 0x57, 0x2E, 0xFF};
const SDL_Color SCORE_COLOR = {0x66, 0x9B, 0xBC, 0xFF};
const SDL_Color SCORE_BARCOLOR = {0x00, 0x00, 0x00, 0x00};
const SDL_Color red = {0xFF, 0x00, 0x00, 0xFF};
const SDL_Color yellow = {0xFF, 0xFF, 0x00, 0xFF};
const SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
const SDL_Color green = {0x00, 0xFF, 0x00, 0xFF};

const int SNAKE_SPEED = 5;
const int SNAKE_SPEEDMS = 50;

#define WINDOW_TITLE "Slidderboa"
#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define FONT_SIZE 60
#define SNAKE_BODY_WIDTH 30
#define SNAKE_BODY_HEIGHT 30
#define SNAKE_FOOD_WIDTH 50
#define SNAKE_FOOD_HEIGHT 50
#define SNAKE_SEGMENT_SPACING 5
#define COLOR_TOPARAM(color) color.r, color.g, color.b, color.a

void slidderboa_game_create(slidderboa_t* game) {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL\n");
        return;
    }
    if(TTF_Init() < 0) {
        fprintf(stderr, "Failed to initialize SDL ttf\n");
    }
    if(Mix_Init(MIX_INIT_MP3) != MIX_INIT_MP3) {
        fprintf(stderr, "Failed to initialize SDL mixer\n");
        return;
    }
    if(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != (IMG_INIT_PNG | IMG_INIT_JPG)) {
        fprintf(stderr, "Failed to initialize SDL image");
        return;
    }
    // Initialize random number generator
    srand(time(NULL));

    // Initialize certain flags in t strucutre
    game->win_width = WINDOW_WIDTH;
    game->win_height = WINDOW_HEIGHT;
    game->window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    game->font = TTF_OpenFont("Roboto-Regular.ttf", FONT_SIZE);

    // Initialize the asset manager and load the default assets
    slidderboa_assetmanager_init(game);
    slidderboa_assetmanager_loadassets(game);

    // Initialize snake properties
    game->food.w = SNAKE_FOOD_WIDTH, game->food.h = SNAKE_FOOD_HEIGHT;
    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONRIGHT;
    slidderboa_game_createsnake_head(game);
}

void slidderboa_game_getwindowsize(slidderboa_t* game) {
    SDL_GetWindowSize(game->window, &game->win_width, &game->win_height);
}

void slidderboa_game_setwindowcolor(slidderboa_t* game, const SDL_Color window_color) {
    SDL_SetRenderDrawColor(game->renderer, COLOR_TOPARAM(window_color));
    SDL_RenderClear(game->renderer);
}

void slidderboa_game_createsnake_head(slidderboa_t* game) {
    game->snake = malloc(sizeof(SDL_Rect));
    game->snake_bodysegment_count = 1;
    slidderboa_game_initializesnake_bodysegment(game, (WINDOW_WIDTH - SNAKE_BODY_WIDTH) / 2, (WINDOW_HEIGHT -
        SNAKE_BODY_HEIGHT) / 2, 0);
    game->snake_created = true;
}

void slidderboa_game_initializesnake_bodysegment(slidderboa_t* game, int x, int y, size_t snakebody_segmentindex) {
    game->snake[snakebody_segmentindex].x = x;
    game->snake[snakebody_segmentindex].y = y;
    game->snake[snakebody_segmentindex].w = SNAKE_BODY_WIDTH;
    game->snake[snakebody_segmentindex].h = SNAKE_BODY_HEIGHT;
}

void slidderboa_game_increasesnake_size(slidderboa_t* game) {
    SDL_Rect *new_ptr = NULL;
    new_ptr = realloc(game->snake, (game->snake_bodysegment_count+1) * sizeof(SDL_Rect));
    if(!new_ptr) {
        fprintf(stderr, "Failed to reallocate memory for the snake\n");
        return;
    }
    game->snake = new_ptr;
    slidderboa_game_initializesnake_bodysegment(game, 0, 0, game->snake_bodysegment_count);
    game->score++;
    game->generated_food = false;
    game->snake_bodysegment_count++;
}

void slidderboa_game_run(slidderboa_t* game) {
    while(!game->quit) {
        slidderboa_game_handle_events(game);
        slidderboa_game_getwindowsize(game);
        slidderboa_game_renderpresent(game);
        game->mouse_clicked = false;
    }
}

bool slidderboa_game_rect_hover(slidderboa_t* game, SDL_Rect rect) {
    if((game->mouse_x <= rect.x + rect.w && game->mouse_x >= rect.x) &&
        (game->mouse_y <= rect.y + rect.h && game->mouse_y >= rect.y)) {
        return true;
    }
    return false;
}

void slidderboa_game_reset(slidderboa_t* game) {
    free(game->snake);
    slidderboa_game_createsnake_head(game);
    if(game->score > game->highscore) {
        game->highscore = game->score;
    }
    game->score = 0;
    game->snake_moving = false;
    game->snake_died = false;
}

void slidderboa_game_displaygame_over(slidderboa_t* game) {
    slidderboa_game_setwindowcolor(game, GAMEOVER_BGCOLOR);
    const char* game_overtext = "Game Over!";
    SDL_Rect game_overcanvas = {0};
    TTF_SetFontSize(game->font, 40);
    TTF_SizeText(game->font, game_overtext, &game_overcanvas.w, &game_overcanvas.h);
    SDL_Surface* game_oversurface = TTF_RenderText_Blended(game->font, game_overtext, red);
    SDL_Texture* game_overtexture = SDL_CreateTextureFromSurface(game->renderer, game_oversurface);
    SDL_FreeSurface(game_oversurface);

    size_t score_len = slidderboa_game_getsize_tlength(game->score),
           highscore_len = slidderboa_game_getsize_tlength(game->highscore),
           finalscore_textlen = 13 + score_len;
    
    SDL_Rect finalscore_canvas = {0};
    char* finalscore_text = malloc(finalscore_textlen + 1);
    sprintf(finalscore_text, "Final Score: %zu", game->score);
    finalscore_text[finalscore_textlen] = '\0';
    TTF_SetFontSize(game->font, 25);
    TTF_SizeText(game->font, finalscore_text, &finalscore_canvas.w, &finalscore_canvas.h);
    SDL_Surface* finalscore_surface = TTF_RenderText_Blended(game->font, finalscore_text, yellow);
    SDL_Texture* finalscore_texture = SDL_CreateTextureFromSurface(game->renderer, finalscore_surface);
    SDL_FreeSurface(finalscore_surface);

    SDL_Rect highscore_canvas = {0};
    size_t highscore_textlen = 0;
    char* highscore_text = NULL;
    printf("game->score: %zu, game->highscore: %zu\n",
        game->score, game->highscore);
    if(game->score > game->highscore) {
        highscore_len = score_len;
        highscore_textlen = highscore_len + 16;
        highscore_text = malloc(highscore_textlen + 1);
        sprintf(highscore_text, "New High Score: %zu", game->score);
        highscore_text[highscore_textlen] = '\0';
    } else {
        highscore_textlen = highscore_len + 12;
        highscore_text = malloc(highscore_textlen + 1);
        sprintf(highscore_text, "High Score: %zu", game->highscore);
        highscore_text[highscore_textlen] = '\0';
    }
    TTF_SizeText(game->font, highscore_text, &highscore_canvas.w, &highscore_canvas.h);
    SDL_Surface* highscore_surface = TTF_RenderText_Blended(game->font, highscore_text, yellow);
    SDL_Texture* highscore_texture = SDL_CreateTextureFromSurface(game->renderer, highscore_surface);
    SDL_FreeSurface(highscore_surface);

    TTF_SetFontSize(game->font, 40);
    const char* playbtn_text = "PLAY AGAIN";
    SDL_Rect playbtn_textcanvas = {0}, playbtn_canvas = {0};
    TTF_SizeText(game->font, playbtn_text, &playbtn_textcanvas.w, &playbtn_textcanvas.h);
    playbtn_canvas.w = playbtn_textcanvas.w + 10, playbtn_canvas.h = playbtn_textcanvas.h + 10;
    SDL_Surface* playbtn_surface = TTF_RenderText_Blended(game->font, playbtn_text, yellow);
    SDL_Texture* playbtn_texture = SDL_CreateTextureFromSurface(game->renderer, playbtn_surface);
    SDL_FreeSurface(playbtn_surface);
    
    int max_width = game_overcanvas.w;
    if(finalscore_canvas.w > max_width) {
        max_width = finalscore_canvas.w;
    }
    if(highscore_canvas.w > max_width) {
        max_width = finalscore_canvas.w;
    }
    if(playbtn_canvas.w > max_width) {
        max_width = finalscore_canvas.w;
    }
    SDL_Rect full_canvas = {
        .x = 0, .y = 0,
        .w = max_width,
        .h = game_overcanvas.h + finalscore_canvas.h + highscore_canvas.h
            + playbtn_canvas.h + 30
    };
    //full_canvas.x = (game->win_width - full_canvas.w) / 2;
    full_canvas.y = (game->win_height - full_canvas.h) / 2;
    game_overcanvas.x = (game->win_width - game_overcanvas.w) / 2;
    game_overcanvas.y = full_canvas.y;
    SDL_RenderCopy(game->renderer, game_overtexture, NULL, &game_overcanvas);
    SDL_DestroyTexture(game_overtexture);

    finalscore_canvas.x = (game->win_width - finalscore_canvas.w) / 2;
    finalscore_canvas.y = game_overcanvas.y + game_overcanvas.h + 10;
    SDL_RenderCopy(game->renderer, finalscore_texture, NULL, &finalscore_canvas);
    SDL_DestroyTexture(finalscore_texture);
    free(finalscore_text);

    highscore_canvas.x = (game->win_width - highscore_canvas.w) / 2;
    highscore_canvas.y = finalscore_canvas.y + finalscore_canvas.h + 10;
    SDL_RenderCopy(game->renderer, highscore_texture, NULL, &highscore_canvas);
    SDL_DestroyTexture(highscore_texture);
    free(highscore_text);

    playbtn_canvas.x = (game->win_width - playbtn_canvas.w) / 2;
    playbtn_canvas.y = highscore_canvas.y + highscore_canvas.h + 10;
    playbtn_textcanvas.x = playbtn_canvas.x + (playbtn_canvas.w - playbtn_textcanvas.w) / 2;
    playbtn_textcanvas.y = playbtn_canvas.y + (playbtn_canvas.h - playbtn_textcanvas.h) / 2;
    SDL_SetRenderDrawColor(game->renderer, COLOR_TOPARAM(green));
    SDL_RenderDrawRect(game->renderer, &playbtn_canvas);
    SDL_RenderCopy(game->renderer, playbtn_texture, NULL, &playbtn_textcanvas);
    SDL_DestroyTexture(playbtn_texture);

    if(slidderboa_game_rect_hover(game, playbtn_canvas)) {
        if(game->mouse_clicked) {
            slidderboa_game_reset(game);
            printf("You clicked the play button\n");
            game->mouse_clicked = false;
        }
    }
}

void slidderboa_game_renderpresent(slidderboa_t* game) {
    if(game->snake_died) {
        slidderboa_game_displaygame_over(game);
    } else {
        slidderboa_game_setwindowcolor(game, WINDOW_COLOR);
        slidderboa_game_renderscore(game);
        slidderboa_game_generatefood(game);
        slidderboa_game_rendersnake(game);
        slidderboa_game_handlecollision(game);
        slidderboa_game_handlemovement(game);
    }
    SDL_RenderPresent(game->renderer);
}

void slidderboa_game_handle_events(slidderboa_t* game) {
    while(SDL_PollEvent(&game->e)) {
        if(game->e.type == SDL_QUIT) {
            game->quit = 1; break;
        } else if(game->e.type == SDL_KEYDOWN) {
            switch(game->e.key.keysym.sym) {
                case SDLK_UP:
                    if(game->snake_direction == SLIDDERBOA_SNAKEDIRECTIONDOWN &&
                        game->snake_bodysegment_count > 1) {
                        break;
                    }
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONUP;
                    game->snake_moving = true;
                    break;
                case SDLK_DOWN:
                    if(game->snake_direction == SLIDDERBOA_SNAKEDIRECTIONUP &&
                        game->snake_bodysegment_count > 1) {
                        break;
                    }
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONDOWN;
                    game->snake_moving = true;
                    break;
                case SDLK_LEFT:
                    if(game->snake_direction == SLIDDERBOA_SNAKEDIRECTIONRIGHT &&
                        game->snake_bodysegment_count > 1) {
                            break;
                    }
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONLEFT;
                    game->snake_moving = true;
                    break;
                case SDLK_RIGHT:
                    if(game->snake_direction == SLIDDERBOA_SNAKEDIRECTIONLEFT &&
                        game->snake_bodysegment_count) {
                        break;
                    }
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONRIGHT;
                    game->snake_moving = true;
                    break;
            }
        } else if(game->e.type == SDL_MOUSEMOTION) {
            game->mouse_x = game->e.motion.x,
            game->mouse_y = game->e.motion.y;
        } else if(game->e.type == SDL_MOUSEBUTTONUP) {
            game->mouse_x = game->e.button.x,
            game->mouse_y = game->e.button.y;
            game->mouse_clicked = true;
        }
    }
}

void slidderboa_game_generatefood(slidderboa_t* game) {
    if(game->generated_food) {
        SDL_RenderCopy(game->renderer, game->food_texture, NULL, &game->food);
        return;
    }
    SDL_Rect* snake = game->snake;
    int food_count = game->asset_manager.assets[0].texture_count,
        food_choice = rand() % food_count, food_x = 0, food_y = 0;
    game->food_texture = game->asset_manager.assets[0].textures[food_choice];
    // Generate the x / horizontal position for the food
    do {
        food_x = rand() % (WINDOW_WIDTH - game->food.w);
    } while(food_x <= snake[0].x + snake[0].w + 5 && food_x >= snake[0].x);
    
    // Generate the y / vertical position for the food
    do {
        food_y = game->score_bar.y + game->score_bar.h +
            (rand() % (WINDOW_HEIGHT - game->score_bar.h - game->food.h - 2));
    } while(food_y <= snake[0].y + snake[0].h && food_y >= snake[0].y);
    game->food.x = food_x, game->food.y = food_y;
    game->generated_food = true;
}

void slidderboa_game_rendersnake(slidderboa_t* game) {
    SDL_SetRenderDrawColor(game->renderer, COLOR_TOPARAM(SNAKE_COLOR));
    for(size_t i=0;i<game->snake_bodysegment_count;i++) {
        SDL_Rect* current_segment = &game->snake[i];
        SDL_RenderDrawRect(game->renderer, current_segment);
        SDL_RenderFillRect(game->renderer, current_segment);

    }
}

int slidderboa_game_getsize_tlength(size_t number) {
    int length = 1;
    while(number >= 10) {
        length++;
        number /= 10;
    }
    return length;
}

char* slidderboa_game_sizet_tostring(size_t number, size_t* number_lengthptr) {
    size_t number_length = slidderboa_game_getsize_tlength(number);
    char* number_string = malloc(number_length+1);
    if(!number_string) {
        return NULL;
    }
    number_string[number_length] = '\0';
    sprintf(number_string, "%zu", number);
    *number_lengthptr = number_length;
    return number_string;
}

void slidderboa_game_renderscore(slidderboa_t* game) {
    size_t score_length = 0;
    char* score = slidderboa_game_sizet_tostring(game->score, &score_length);
    SDL_Rect *score_bar = &game->score_bar,
             *score_textcanvas = &game->score_canvas[0],
             *score_canvas = &game->score_canvas[1];
    char* score_text = "Score: ";
    // Calculate the position and dimensions to store the score text and the score within the score bar
    TTF_SizeText(game->font, score_text, &score_textcanvas->w, &score_textcanvas->h);
    TTF_SizeText(game->font, score, &score_canvas->w, &score_canvas->h);
    score_bar->w = game->win_width,
    score_bar->h = (score_textcanvas->h > score_canvas->h) ? score_textcanvas->h : score_canvas->h;
    score_canvas->x = score_textcanvas->x + score_textcanvas->w;

    SDL_SetRenderDrawColor(game->renderer, COLOR_TOPARAM(SCORE_BARCOLOR));
    SDL_RenderDrawRect(game->renderer, score_bar);
    SDL_RenderFillRect(game->renderer, score_bar);

    // Display the score text and free the memory we allocate for it
    SDL_Surface* score_text_surface = TTF_RenderText_Blended(game->font, score_text, SCORETEXT_COLOR);
    SDL_Texture* score_text_texture = SDL_CreateTextureFromSurface(game->renderer, score_text_surface);
    SDL_RenderCopy(game->renderer, score_text_texture, NULL, score_textcanvas);
    SDL_FreeSurface(score_text_surface); score_text_surface = NULL;
    SDL_DestroyTexture(score_text_texture);

    // Display the actual score and free the memory we allocated for it
    SDL_Surface* score_surface = TTF_RenderText_Blended(game->font, score, SCORE_COLOR);
    SDL_Texture* score_texture = SDL_CreateTextureFromSurface(game->renderer, score_surface);
    SDL_RenderCopy(game->renderer, score_texture, NULL, score_canvas);
    SDL_FreeSurface(score_surface);
    SDL_DestroyTexture(score_texture);
    free(score); score = NULL;
}

void slidderboa_game_handlemovement(slidderboa_t* game) {
    if(game->snake_died) {
        return;
    }
    if(!game->snake_moving) {
        return;
    }
    Uint32 current_time = SDL_GetTicks();
    if(!game->snake_deltatime) {
        game->snake_deltatime = SDL_GetTicks() + SNAKE_SPEEDMS;
    } else if(current_time >= game->snake_deltatime) {
        game->snake_deltatime = 0;
    } else {
        return;
    }
    for(size_t i=game->snake_bodysegment_count-1;i>0;i--) {
        game->snake[i].x = game->snake[i-1].x;
        game->snake[i].y = game->snake[i-1].y;
    }
    switch(game->snake_direction) {
        case SLIDDERBOA_SNAKEDIRECTIONUP:
            game->snake[0].y -= SNAKE_SPEED;
            break;
        case SLIDDERBOA_SNAKEDIRECTIONDOWN:
            game->snake[0].y += SNAKE_SPEED;
            break;
        case SLIDDERBOA_SNAKEDIRECTIONLEFT:
            game->snake[0].x -= SNAKE_SPEED;
            break;
        case SLIDDERBOA_SNAKEDIRECTIONRIGHT:
            game->snake[0].x += SNAKE_SPEED;
            break;
    }
}

void slidderboa_game_handlecollision(slidderboa_t* game) {
    slidderboa_game_handlewall_collision(game);
    slidderboa_game_handlefood_collision(game);
}

void slidderboa_game_handlefood_collision(slidderboa_t* game) {
    SDL_Rect* snake = game->snake, food = game->food;
    int snake_direction = game->snake_direction;
    bool snake_ate = false;
    if(!game->snake_moving) {
        return;
    }
    switch(snake_direction) {
        case SLIDDERBOA_SNAKEDIRECTIONUP:
            if(snake[0].x <= food.x + food.w && snake[0].x + snake[0].w >= food.x
                && snake[0].y <= food.y + food.h + 1 && snake[0].y >= food.y) {
                printf("You are in the same x and y position as the food just one more step\n");
                snake_ate = true;
            }
            break;
        case SLIDDERBOA_SNAKEDIRECTIONDOWN:
            if(snake[0].x <= food.x + food.w && snake[0].x + snake[0].w >= food.x
                && snake[0].y + snake[0].h >= food.y) {
                printf("You are in the same x and y position as the food just one more step\n");
                snake_ate = true;
            }
            break;
        case SLIDDERBOA_SNAKEDIRECTIONLEFT:
            if(snake[0].x <= food.x + food.w && snake[0].x >= food.x
                && ((snake[0].y <= food.y + food.h && snake[0].y >= food.y) || (snake[0].y +
                    snake[0].h <= food.y + food.h && snake[0].y + snake[0].h >= food.y))) {
                snake_ate = true;
            }
            break;
        case SLIDDERBOA_SNAKEDIRECTIONRIGHT:
            if(snake[0].x + snake[0].w <= food.x + food.w && snake[0].x + snake[0].w >= food.x
                && ((snake[0].y <= food.y + food.h && snake[0].y >= food.y) || (snake[0].y +
                    snake[0].h <= food.y + food.h && snake[0].y + snake[0].h >= food.y))) {
                snake_ate = true;
            }
            break;
    }
    if(snake_ate) {
        printf("You just ate food\n");
        slidderboa_game_increasesnake_size(game);
    }
}

void slidderboa_game_handlewall_collision(slidderboa_t* game) {
    SDL_Rect* snake = game->snake;
    int snake_direction = game->snake_direction;
    if(game->snake_died) {
        return;
    }
    if(!game->snake_moving) {
        return;
    }
    if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONLEFT && snake[0].x <= 0) {
        printf("The snake collided while moving LEFT\n");
        game->snake_died = true;
    } else if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONRIGHT && snake[0].x + snake[0].w >= game->win_width) {
        printf("The snake collided while moving RIGHT\n");
        game->snake_died = true;
    } else if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONUP && snake[0].y <= game->score_bar.h) {
        printf("The snake collided while moving UP\n");
        game->snake_died = true;
    } else if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONDOWN && snake[0].y + snake[0].h >= game->win_height) {
        printf("The snake collided while moving DOWN\n");
        game->snake_died = true;
    }
}

void slidderboa_game_destroy(slidderboa_t* game) {
    free(game->snake); game->snake = NULL;
    game->snake_bodysegment_count = 0;
    slidderboa_assetmanager_destroy(game);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}