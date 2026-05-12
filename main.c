/**
 * @file main.c
 * @brief Programme principal du jeu.
 * @author Maram Hnaien
 * @version 1
 * @date 2026
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "game.h"

int main() {
    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font     *font     = NULL;

    Enemy    enemy;
    Obstacle obs;
    Snowball snowballs[MAX_SNOWBALLS];
    int      snowballCount = 0;
    Player   player;
    int      selectedSkinIndex = 0;
    Skin skins[MAX_SKINS];
    char skinNames[MAX_SKINS][32];
    char skinPaths[MAX_SKINS][64];
    char idlePaths[MAX_SKINS][64];
    char walkPaths[MAX_SKINS][64];
    char runPaths[MAX_SKINS][64];
    char touchPaths[MAX_SKINS][64];
    char hurtPaths[MAX_SKINS][64];

    Mix_Chunk *collisionSound  = NULL;
    Mix_Music *backgroundMusic = NULL;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Mon Jeu SDL2",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("Erreur SDL_image: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    if (TTF_Init() == -1) {
        printf("Erreur SDL_ttf: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    srand((unsigned int)(time(NULL) ^ SDL_GetPerformanceCounter()));
    font = TTF_OpenFont("font.ttf", 55);
    if (!font) {
        printf("Erreur chargement police: %s\n", TTF_GetError());
    }

    for (int i = 0; i < MAX_SKINS; i++) {
        snprintf(skinNames[i], sizeof(skinNames[i]), "image%d", i + 1);
        if (!resolveAssetPath(skinNames[i], skinPaths[i], sizeof(skinPaths[i]))) {
            snprintf(skinPaths[i], sizeof(skinPaths[i]), "enemy.png");
        }
        skins[i].name = skinNames[i];
        skins[i].normalPath = skinPaths[i];
        snprintf(idlePaths[i], sizeof(idlePaths[i]), "spritesheet%d/0.png", i + 1);
        snprintf(walkPaths[i], sizeof(walkPaths[i]), "spritesheet%d/1.png", i + 1);
        snprintf(runPaths[i], sizeof(runPaths[i]), "spritesheet%d/2.png", i + 1);
        snprintf(hurtPaths[i], sizeof(hurtPaths[i]), "spritesheet%d/3.png", i + 1);
        snprintf(touchPaths[i], sizeof(touchPaths[i]), "spritesheet%d/4.png", i + 1);
        skins[i].idlePath = idlePaths[i];
        skins[i].walkPath = walkPaths[i];
        skins[i].runPath = runPaths[i];
        skins[i].hurtPath = hurtPaths[i];
        skins[i].playerTouchPath = touchPaths[i];
        skins[i].tint = (SDL_Color){255, 255, 255, 255};
    }

    selectedSkinIndex = showSkinSelectionScreen(renderer, font, skins, MAX_SKINS);

 
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    backgroundMusic = Mix_LoadMUS("jingle.mp3");
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    }
    collisionSound = Mix_LoadWAV("sound.wav");

    init(renderer,
         &enemy,(char *)skins[selectedSkinIndex].idlePath,5,5,
         (char *)skins[selectedSkinIndex].hurtPath,5,5,
         (char *)skins[selectedSkinIndex].playerTouchPath,
         (char *)skins[selectedSkinIndex].walkPath,
         (char *)skins[selectedSkinIndex].runPath,
         "tree.png",
         &obs,"falling.png",600,300,
         &snowballs[0],"snowball.png",
         &player,"player.png",5,5);
    setEnemySkin(&enemy, &skins[selectedSkinIndex]);

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

        updatePlayer(&player, &enemy, &obs, snowballs, &snowballCount);
        move(&enemy, &obs, &player);
        checkCollision(renderer, &enemy, &obs, collisionSound,
                       snowballs, &snowballCount, &player);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        scrollBackground(renderer, &enemy, &obs, &player);
        affichage(renderer, &enemy, &obs,
                  snowballs, snowballCount, &player);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    destroyAll(&enemy, &obs, collisionSound, &snowballs[0]);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
