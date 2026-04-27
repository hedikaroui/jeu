#include "game.h"
#include <stdio.h>

int main(void) {
    Game game;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!Initialisation(&game, &window, &renderer)) {
        fprintf(stderr, "Initialisation echouee\n");
        return 1;
    }

    while (game.running) {
        GameLoop_ModuleInput(&game, renderer);
        GameLoop_ModuleUpdate(&game);
        GameLoop_ModuleAffichage(&game, renderer);
    }

    Liberation(&game, window, renderer);
    return 0;
}
