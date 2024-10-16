#include <stdio.h>
#include "slidderboa.h"

int main(int argc, char** argv) {
    slidderboa_t game = {0};
    slidderboa_game_create(&game);
    slidderboa_game_run(&game);
    slidderboa_game_destroy(&game);
    return 0;
}