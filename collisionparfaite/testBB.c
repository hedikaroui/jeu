#include "collision.h"
#include <stdio.h>

#define WIDTH  800
#define HEIGHT 480

int main()
{
    GameBB g;


    if (!InitFenetreBB(&g, "Test Collision Bounding Box", WIDTH, HEIGHT)) return 1;
    if (!InitRendererBB(&g)) return 1;
    if (!LoadBB(&g,
                "assets/background.bmp",
                "assets/joueur.png",
                WIDTH/2, HEIGHT/2, 40, 50)) return 1;

    SDL_Event ev;


    while (g.running) {


        SDL_SetRenderDrawColor(g.renderer, 30, 30, 50, 255);
        SDL_RenderClear(g.renderer);


        SDL_Rect destBG = { 0, 0, WIDTH, HEIGHT };
        if (g.bgTexture)
            SDL_RenderCopy(g.renderer, g.bgTexture, NULL, &destBG);
        else {
            SDL_SetRenderDrawColor(g.renderer, 70, 120, 70, 255);
            SDL_RenderFillRect(g.renderer, &destBG);
        }


        for (int i = 0; i < g.nbPlateformes; i++) {
            if (!g.plateformes[i].active) continue;
            if (g.plateformes[i].destructible)
                SDL_SetRenderDrawColor(g.renderer, 200, 50, 50, 255);  
            else
                SDL_SetRenderDrawColor(g.renderer, 50, 180, 50, 255);  
            SDL_RenderFillRect(g.renderer, &g.plateformes[i].pos);
        }


        SDL_SetRenderDrawColor(g.renderer, 50, 100, 220, 255);
        SDL_RenderFillRect(g.renderer, &g.joueur.pos);
        SDL_RenderPresent(g.renderer);

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                g.running = 0;
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_ESCAPE: g.running = 0;                         break;
                    case SDLK_LEFT:   g.joueur.pos.x -= g.joueur.vitesse;    break;
                    case SDLK_RIGHT:  g.joueur.pos.x += g.joueur.vitesse;    break;
                    case SDLK_UP:     g.joueur.pos.y -= g.joueur.vitesse;    break;
                    case SDLK_DOWN:   g.joueur.pos.y += g.joueur.vitesse;    break;
                }
            }
        }



        if (g.joueur.pos.x < 0)                       g.joueur.pos.x = 0;
        if (g.joueur.pos.y < 0)                       g.joueur.pos.y = 0;
        if (g.joueur.pos.x + g.joueur.pos.w > WIDTH)  g.joueur.pos.x = WIDTH  - g.joueur.pos.w;
        if (g.joueur.pos.y + g.joueur.pos.h > HEIGHT) g.joueur.pos.y = HEIGHT - g.joueur.pos.h;


        SDL_Rect anciennePos = g.joueur.pos;


        for (int i = 0; i < g.nbPlateformes; i++) {
            if (collisionBoundingBox(&g.joueur, &g.plateformes[i])) {
                printf("[BB] Collision avec plateforme %d\n", i);
                if (g.plateformes[i].destructible) {
                    g.plateformes[i].active = 0;
                    printf("[BB] Plateforme %d détruite !\n", i);
                } else {
                    g.joueur.pos = anciennePos; // annule le déplacement
                }
            }
        }



    }

    // Libération
    LibererBB(&g);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
