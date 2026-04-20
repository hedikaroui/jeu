#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "game.h"

int main() {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    Enemy enemy;
    Obstacle spider;
    Obstacle falling;
    HUD hud;

    Mix_Chunk *collisionSound = NULL;

    // ================= INIT SDL =================
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Mon Jeu SDL2",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,800, 600,SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erreur création fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erreur création renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // ================= INIT AUDIO =================
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    collisionSound = Mix_LoadWAV("sound.wav");

    // ================= INIT GAME OBJECTS =================
    initEnemy(renderer, &enemy,"enemy.png", 5, 5,"enemy2.png",5, 2);  
    initObstacle(renderer, &spider, "spider.png", 600, 300);
    initObstacle(renderer, &falling, "falling.png", 400, 200);
    initHUD(&hud, renderer, "tree.png");
    initBackground(&hud, renderer, "background1.png");

    // ================= GAME LOOP =================
    int running = 1;
    SDL_Event event;

    while (running) {
        // ---- Events ----
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        // ---- Update ----
        moveAndAnimateEnemy(&enemy, &hud);
        moveObstacle(&spider);
        moveObstacle(&falling);
        
        // Collisions obstacles
        checkCollision(renderer, &enemy, &falling, collisionSound);
        checkCollision(renderer, &enemy, &spider, collisionSound);
        
        // Collision joueur avec ennemi (appeler ici si joueur existe)
        // checkEnemyHit(renderer, &enemy, &hud, "enemy2.png");

        // ---- Render ----
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderBackground(renderer, &hud);
        renderEnemy(renderer, &enemy);
        renderObstacle(renderer, &spider);
        renderObstacle(renderer, &falling);
        renderHUD(renderer, &hud);

        SDL_RenderPresent(renderer);
    }

    // ================= CLEANUP =================
    destroyAll(&enemy, &spider, &falling, collisionSound, &hud);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
