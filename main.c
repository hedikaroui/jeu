#include "minimap.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    SDL_Window   *window   = InitFenetre("Minimap", WIDTH, HEIGHT);
    if (!window) return 1;
    SDL_Renderer *renderer = InitRenderer(window);
    if (!renderer) { SDL_DestroyWindow(window); return 1; }

    Minimap m;
    if (!LoadRessources(&m, renderer,
                        "assets/background.png", "assets/mario.png",
                        WIDTH-180, 0, 180, 120, 16, 16, 20)) {
        printf("Erreur LoadRessources\n"); return 1;
    }

    SDL_Surface *maskSurf = SDL_LoadBMP("assets/backgroundmasq.bmp");
    if (!maskSurf) fprintf(stderr, "[PP] Masque introuvable\n");

    Entite entite;
    entite.texture = IMG_LoadTexture(renderer, "assets/mario2.jpg");
    entite.pos     = (SDL_Rect){ 400, 250, 50, 60 };

    Etincelle etincelle;
    LoadEtincelle(&etincelle, renderer, "assets/anim.png", 5, 2);

    GameState state;
    initGameState(&state, (SDL_Rect){ 100, 100, 50, 60 }, 0.0f,
                  1.0f);

    Uint32 lastTick = SDL_GetTicks();

    while (m.running) {

        Uint32 now   = SDL_GetTicks();
        float  delta = (now - lastTick) / 1000.0f;
        lastTick     = now;

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
            state.posJoueur = new_pos;
            m.playerPosition.x = m.minimapPosition.x + (new_pos.x * m.redimensionnement) / 100;
            m.playerPosition.y = m.minimapPosition.y + (new_pos.y * m.redimensionnement) / 100;
        } else {
            state.collisionBBEvent = bb;
            state.collisionPPEvent = pp;
            state.borderTimer = 0.5f;
            declencherEtincelle(&etincelle, m.playerPosition, 0);
        }

        state.time = now;

        if (state.borderTimer > 0.0f) {
            state.borderTimer -= delta;
            if (state.borderTimer < 0.0f) state.borderTimer = 0.0f;
        }

        updateEtincelle(&etincelle, delta);

    }

    liberer(&etincelle, &entite, maskSurf, &m);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
