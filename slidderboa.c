#include "slidderboa.h"
#include "slidderboa_assetmanager.h"

const SDL_Color WINDOW_COLOR = {0x29, 0x33, 0x5C, 0xFF};
const SDL_Color SNAKE_COLOR = {0xF3, 0xA7, 0x12, 0xFF};
const SDL_Color SCORETEXT_COLOR = {0xE4, 0x57, 0x2E, 0xFF};
const SDL_Color SCORE_COLOR = {0x66, 0x9B, 0xBC, 0xFF};
const SDL_Color SCORE_BARCOLOR = {0x00, 0x00, 0x00, 0x00};

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
    }
}

void slidderboa_game_renderpresent(slidderboa_t* game) {
    slidderboa_game_setwindowcolor(game, WINDOW_COLOR);
    slidderboa_game_renderscore(game);
    slidderboa_game_generatefood(game);
    slidderboa_game_rendersnake(game);
    slidderboa_game_handlemovement(game);
    slidderboa_game_handlecollision(game);
    SDL_RenderPresent(game->renderer);
}

void slidderboa_game_handle_events(slidderboa_t* game) {
    while(SDL_PollEvent(&game->e)) {
        if(game->e.type == SDL_QUIT) {
            game->quit = 1; break;
        } else if(game->e.type == SDL_KEYDOWN) {
            switch(game->e.key.keysym.sym) {
                case SDLK_UP:
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONUP;
                    game->snake_moving = true;
                    break;
                case SDLK_DOWN:
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONDOWN;
                    game->snake_moving = true;
                    break;
                case SDLK_LEFT:
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONLEFT;
                    game->snake_moving = true;
                    break;
                case SDLK_RIGHT:
                    game->snake_direction = SLIDDERBOA_SNAKEDIRECTIONRIGHT;
                    game->snake_moving = true;
                    break;
            }
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
        if(i != game->snake_bodysegment_count-1) {
            switch(game->snake_direction) {
                case SLIDDERBOA_SNAKEDIRECTIONUP:
                    game->snake[i+1].x = game->snake[i].x;
                    game->snake[i+1].y = game->snake[i].y + game->snake[i].h;
                    break;
                case SLIDDERBOA_SNAKEDIRECTIONDOWN:
                    game->snake[i+1].x = game->snake[i].x;
                    game->snake[i+1].y = game->snake[i].y + game->snake[i].h;
                    break;
                case SLIDDERBOA_SNAKEDIRECTIONLEFT:
                    game->snake[i+1].x = game->snake[i].x + game->snake[i].w;
                    game->snake[i+1].y = game->snake[i].y;
                    break;
                case SLIDDERBOA_SNAKEDIRECTIONRIGHT:
                    game->snake[i+1].x = game->snake[i].x + game->snake[i].w;
                    game->snake[i+1].y = game->snake[i].y;
                    break;
            }
        }
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
    switch(game->snake_direction) {
        case SLIDDERBOA_SNAKEDIRECTIONUP:
            if(game->snake[0].y > game->snake[0].h) {
                game->snake[0].y -= (int)SNAKE_SPEED;
            }
            break;
        case SLIDDERBOA_SNAKEDIRECTIONDOWN:
            if(game->snake[0].y < game->win_height - game->snake[0].h) {
                game->snake[0].y += (int)SNAKE_SPEED;
            }
            break;
        case SLIDDERBOA_SNAKEDIRECTIONLEFT:
            if(game->snake[0].x > 0) {
                game->snake[0].x -= (int)SNAKE_SPEED;
            }
            break;
        case SLIDDERBOA_SNAKEDIRECTIONRIGHT:
            if(game->snake[0].x < game->win_width - game->snake[0].w) {
                game->snake[0].x += (int)SNAKE_SPEED;
            }
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
    if(!game->snake_moving) {
        return;
    }
    switch(snake_direction) {
        case SLIDDERBOA_SNAKEDIRECTIONUP:
            if(snake[0].y+1 <= food.y + food.h && snake[0].y+1 >= food.y) {
                slidderboa_game_increasesnake_size(game);
            }
            break;
        /*case SLIDDERBOA_SNAKEDIRECTIONLEFT:
            if(snake[0].x-1 <= food.x + food.w && snake[0].x-1 >= food.x) {
                slidderboa_game_increasesnake_size(game);
            }
            break;
        case SLIDDERBOA_SNAKEDIRECTIONRIGHT:
            if((snake[0].x+1 <= food.x + food.w && snake[0].x >= food.x) &&
                (snake[0].y+1 <= food.y + food.h && snake[0].y+1 >= food.y)) {
                slidderboa_game_increasesnake_size(game);
            }
            break;
        */
    }
}

void slidderboa_game_handlewall_collision(slidderboa_t* game) {
    SDL_Rect* snake = game->snake;
    int snake_direction = game->snake_direction;
    if(!game->snake_moving) {
        return;
    }
    if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONLEFT && snake[0].x <= 0) {
        printf("The snake collided while moving LEFT\n");
    } else if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONRIGHT && snake[0].x + snake[0].h >= game->win_width) {
        printf("The snake collided while moving RIGHT\n");
    } else if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONUP && snake[0].y <= game->score_bar.h) {
        printf("The snake collided while moving UP\n");
    } else if(snake_direction == SLIDDERBOA_SNAKEDIRECTIONDOWN && snake[0].y + snake[0].h >= game->win_height) {
        printf("The snake collided while moving DOWN\n");
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