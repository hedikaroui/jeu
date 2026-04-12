#include "collision.h"
#include <stdio.h>

#define WIDTH  800
#define HEIGHT 480

int main()
{
    GamePixel g;


    if (!InitFenetrePixel(&g, "Test Collision Parfaite", WIDTH, HEIGHT)) return 1;
    if (!InitRendererPixel(&g)) return 1;
    if (!LoadPixel(&g,
                   "assets/background.bmp",
                   "assets/backgroundmasq.bmp",
                   "assets/joueur.png",
                   WIDTH/2, HEIGHT/2, 40, 50)) return 1;

    SDL_Event ev;
    int collision = 0;


    while (g.running) {


        SDL_SetRenderDrawColor(g.renderer, 30, 30, 50, 255);
        SDL_RenderClear(g.renderer);


        SDL_Rect destBG = { 0, 0, WIDTH, HEIGHT };
        if (g.bgTexture)
            SDL_RenderCopy(g.renderer, g.bgTexture, NULL, &destBG);


        if (collision)
            SDL_SetRenderDrawColor(g.renderer, 220, 50, 50, 255);
        else
            SDL_SetRenderDrawColor(g.renderer, 50, 100, 220, 255);
        SDL_RenderFillRect(g.renderer, &g.joueur.pos);
        SDL_RenderPresent(g.renderer);


        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                g.running = 0;
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
                g.running = 0;
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);


        SDL_Rect anciennePos = g.joueur.pos;


        if (keys[SDL_SCANCODE_LEFT])  g.joueur.pos.x -= g.joueur.vitesse;
        if (keys[SDL_SCANCODE_RIGHT]) g.joueur.pos.x += g.joueur.vitesse;
        if (keys[SDL_SCANCODE_UP])    g.joueur.pos.y -= g.joueur.vitesse;
        if (keys[SDL_SCANCODE_DOWN])  g.joueur.pos.y += g.joueur.vitesse;


        if (g.joueur.pos.x < 0)                        g.joueur.pos.x = 0;
        if (g.joueur.pos.y < 0)                        g.joueur.pos.y = 0;
        if (g.joueur.pos.x + g.joueur.pos.w > WIDTH)   g.joueur.pos.x = WIDTH  - g.joueur.pos.w;
        if (g.joueur.pos.y + g.joueur.pos.h > HEIGHT)  g.joueur.pos.y = HEIGHT - g.joueur.pos.h;


        collision = collisionPixelPerfect(&g);
        if (collision) {
            g.joueur.pos = anciennePos; 
            printf("[PP] Collision détectée !\n");
        }



    }


    LibererPixel(&g);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
