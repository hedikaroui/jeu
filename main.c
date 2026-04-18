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
        GameLoop_ModuleInitialisationEtat(&game, renderer); // a deplacer 
        GameLoop_ModuleInput(&game);
        GameLoop_ModuleUpdate(&game);
        GameLoop_ModuleAffichage(&game, renderer);

        if (game.currentSubState == STATE_QUIT) {
            game.running = 0;
        }
    }

    Liberation(&game, window, renderer);
    return 0;
}
