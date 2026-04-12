#include "collision.h"
#include <stdio.h>



int main()
{
    GamePixel gp;
    GameBB gb;
    SDL_Window   *window = NULL;
    SDL_Renderer *renderer = NULL;
    int mode      = MODE_BB;
    int collision = 0;
    SDL_Event ev;
    SDL_Rect destBG = {0, 0, WIDTH, HEIGHT};

    int gauche = 0, droite = 0, haut = 0, bas = 0;

    if (!InitSDLWindowRenderer(&window, &renderer, "Test Collision", WIDTH, HEIGHT)) {
      return 1;
    }

    
    
    InitGamePixel(&gp, renderer);



    InitGameBB(&gb, renderer);


    while (gb.running == 1) {
    	afficherJeu(&gb, &gp, renderer, mode, collision);

        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) gb.running = 0;
            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                    case SDLK_ESCAPE: gb.running = 0; break;
                    case SDLK_LEFT:   gauche = 1; break;
                    case SDLK_RIGHT:  droite = 1; break;
                    case SDLK_UP:     haut = 1; break;
                    case SDLK_DOWN:   bas = 1; break;
                    case SDLK_t:
                    if (mode == MODE_BB) {
    			mode = MODE_PIXEL;
    			} else {
    			 mode = MODE_BB;
    			 }

                        collision = 0;
                        if (mode == MODE_BB) printf("Mode : Bounding Box\n");
                        else printf("Mode : Pixel Perfect\n");
                        break;
                }
            }
            if (ev.type == SDL_KEYUP) {
                switch (ev.key.keysym.sym) {
                    case SDLK_LEFT:   gauche = 0; break;
                    case SDLK_RIGHT:  droite = 0; break;
                    case SDLK_UP:     haut = 0; break;
                    case SDLK_DOWN:   bas = 0; break;
                }
            }
        }


        updateJeu(&gb, &gp, mode, &collision, gauche, droite, haut, bas);



    }

    Cleanup(&gb, &gp, renderer, window);

    return 0;
}
