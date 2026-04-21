#include "game.h"
#include <stdio.h>

int initialize_game(Game *game, SDL_Window **window, SDL_Renderer **renderer) {
    if (!Initialisation(game, window, renderer)) {
        fprintf(stderr, "Initialisation echouee\n");
        return 0;
    }
    return 1;
}

void run_game_loop(Game *game, SDL_Renderer *renderer) {
    while (game->running) {
        GameLoop_ModuleInput(game, renderer);
        GameLoop_ModuleUpdate(game);
        GameLoop_ModuleAffichage(game, renderer);
    }
}

int main(void) {
    Game game;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!initialize_game(&game, &window, &renderer)) {
        return 1;
    }

    run_game_loop(&game, renderer);
    Liberation(&game, window, renderer);
    return 0;
}
