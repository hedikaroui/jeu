#include "minimap.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;
    if (!InitSDL(&window, &renderer, "Minimap", WIDTH, HEIGHT))
        return 1;

    Minimap m;
    GameState state;
    if (!LoadRessources(&m, &state, renderer,
                        "assets/background.png", "assets/mario.png",
                        WIDTH-180, 0, 180, 120, 16, 16, 20,
                        (SDL_Rect){ 100, 100, 50, 60 }, 0.0f, 1.0f)) {
        printf("Erreur LoadRessources\n");
        return 1;
    }

    SDL_Surface *maskSurf = SDL_LoadBMP("assets/backgroundmasq.bmp");
    if (!maskSurf) fprintf(stderr, "[PP] Masque introuvable\n");

    Entite entite;
    entite.texture = IMG_LoadTexture(renderer, "assets/mario2.jpg");
    entite.pos     = (SDL_Rect){ 400, 250, 50, 60 };

    Etincelle etincelle;
    LoadEtincelle(&etincelle, renderer, "assets/anim.png", 5, 2);

    while (m.running) {
        afficherMinimap(&m, renderer, &state, &entite);
        afficherEtincelle(renderer, &etincelle);
        renderBorder(renderer, m.minimapPosition, state.borderTimer);
        SDL_RenderPresent(renderer);

        Lecture(&m, &state);

        SDL_Rect new_pos = state.posJoueur;
        UpdateGame(&new_pos, state.gauche, state.droite, state.haut, state.bas,
                   &state.rotation, &m);

        int bb = collisionBB(new_pos, entite.pos);
        int pp = collisionPP(maskSurf, new_pos);

        if (!bb && !pp) {
            state.posJoueur    = new_pos;
            m.playerPosition.x = m.minimapPosition.x +
                                 (new_pos.x * m.redimensionnement) / 100;
            m.playerPosition.y = m.minimapPosition.y +
                                 (new_pos.y * m.redimensionnement) / 100;
        } else {
            state.collisionBBEvent = bb;
            state.collisionPPEvent = pp;
            state.borderTimer      = 30;
            declencherEtincelle(&etincelle, m.playerPosition, 0);
        }

        if (state.borderTimer > 0)
            state.borderTimer--;

        updateEtincelle(&etincelle);
    }

    Liberation(&etincelle, &entite, maskSurf, &m);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
