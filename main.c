#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "game.h"

int main() {
    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;

    Enemy    enemy;
    Obstacle obs;
    Snowball snowballs[MAX_SNOWBALLS];
    int      snowballCount = 0;
    Player   player;

    Mix_Chunk *collisionSound  = NULL;
    Mix_Music *backgroundMusic = NULL;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Mon Jeu SDL2",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erreur fenetre: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    /* ================= INIT AUDIO ================= */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    backgroundMusic = Mix_LoadMUS("jingle.mp3");
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    }
    collisionSound = Mix_LoadWAV("sound.wav");

    init(renderer,
         &enemy,"enemy.png",5,5,
         "enemy2.png",5,5,"tree.png",
         &obs,"falling.png",600,300,
         &snowballs[0],"snowball.png",
         &player,"player.png",5,5);

    for (int i = 1; i < MAX_SNOWBALLS; i++) {
        snowballs[i].texture = snowballs[0].texture;
        snowballs[i].rect.w  = snowballs[0].rect.w;
        snowballs[i].rect.h  = snowballs[0].rect.h;
        snowballs[i].active  = 0;
    }

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            handleInput(&event, &player, snowballs, &snowballCount);
        }

        move(&enemy, &obs);
        updatePlayer(&player, &enemy, snowballs, &snowballCount);
        checkCollision(renderer, &enemy, &obs, collisionSound,
                       snowballs, &snowballCount, &player);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        scrollBackground(renderer, &enemy, &obs);
        affichage(renderer, &enemy, &obs,
                  snowballs, snowballCount, &player);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    destroyAll(&enemy, &obs, collisionSound, &snowballs[0]);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

