#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include "header.h"

int main(int argc, char *argv[])
{
    SaveGame game;
    
    if (!InitSDL(&game)) {
    printf("Erreur InitSDL\n");
    return 1;
}

    game.window = SDL_CreateWindow("Sauvegarde",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!game.window) {
        printf("Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        return 1;
    }

    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    if (!game.renderer) {
        game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_SOFTWARE);
        if (!game.renderer) {
            printf("Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
            return 1;
        }
    }

    if (!Initialisation(&game)) {
        printf("Erreur Initialisation\n");
        return 1;
    }

    if (!Load(&game)) {
        printf("Erreur Load\n");
        return 1;
    }

    while (game.running)
    {
        Affichage    (&game);
        LectureEntree(&game);
        MiseAJour    (&game);
    }

    Liberation(&game);
    return 0;
}
