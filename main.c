#include "header.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    enigme e;
    int running = 1;
    if (!InitSDL(&e)) return 1;
    e.window = SDL_CreateWindow("Quiz",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!e.window) {
        printf("Erreur fenetre : %s\n", SDL_GetError());
        return 1;
    }
    e.renderer = SDL_CreateRenderer(e.window, -1, SDL_RENDERER_ACCELERATED);
    if (!e.renderer) {
        printf("Erreur renderer : %s\n", SDL_GetError());
        return 1;
    }
    if (!Initialisation(&e)) return 1;
    if (!Load(&e)) { Liberation(&e); return 1; }
    while (running)
    {
        Affichage(&e);
        GererEvenement(&e, &running);
        MiseAJour(&e);
        SDL_Delay(16);
    }
    Liberation(&e);
    return 0;
}
