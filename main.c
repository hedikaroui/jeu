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

    DangerZone zones[2] = {
        { { 200, 150, 100, 80 }, 0.9f, 1 },
        { { 500, 350, 120, 60 }, 0.4f, 1 }
    };

    GameState state;
    initGameState(&state, (SDL_Rect){ 100, 100, 50, 60 }, 0.0f,
                  zones, 2, 1.0f);

    int    gauche = 0, droite = 0, haut = 0, bas = 0;
    Uint32 lastTick = SDL_GetTicks();
    SDL_Event e;

    while (m.running) {

        /* Delta time */
        Uint32 now   = SDL_GetTicks();
        float  delta = (now - lastTick) / 1000.0f;
        lastTick     = now;

        /* ── RENDER ── */
        afficherMinimap(&m, renderer, &state, &entite);
        afficherEtincelle(renderer, &etincelle);
        SDL_RenderPresent(renderer);

        /* ── INPUT ── */
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) m.running = 0;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE: m.running = 0; break;
                    case SDLK_LEFT:   gauche = 1;     break;
                    case SDLK_RIGHT:  droite = 1;     break;
                    case SDLK_UP:     haut   = 1;     break;
                    case SDLK_DOWN:   bas    = 1;     break;
                    /* Zoom : Z = in, X = out */
                    case SDLK_z:
                        state.zoom += ZOOM_STEP;
                        if (state.zoom > ZOOM_MAX) state.zoom = ZOOM_MAX;
                        break;
                    case SDLK_x:
                        state.zoom -= ZOOM_STEP;
                        if (state.zoom < ZOOM_MIN) state.zoom = ZOOM_MIN;
                        break;
                    default: break;
                }
            }
            if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:  gauche = 0; break;
                    case SDLK_RIGHT: droite = 0; break;
                    case SDLK_UP:    haut   = 0; break;
                    case SDLK_DOWN:  bas    = 0; break;
                    default: break;
                }
            }
        }

        /* ── PHYSICS ── */
        SDL_Rect ancienne = state.posJoueur;
        UpdateGame(&state.posJoueur, gauche, droite, haut, bas,
                   &state.rotation, &m);

        /* ── COLLISION DETECTION ── */
        checkCollisionBB(&state.posJoueur, ancienne, &m, &entite, &state);
        checkCollisionPP(&state.posJoueur, ancienne, &m, maskSurf, &state);

        /* ── EVENT DISPATCH ── */
        if (state.collisionBBEvent || state.collisionPPEvent) {
            state.borderTimer = 0.5f;   /* bordure rouge 0.5s */
            declencherEtincelle(&etincelle, m.playerPosition, 0);
        }

        /* ── GAME STATE UPDATE ── */
        state.time = now;

        /* Décrément borderTimer */
        if (state.borderTimer > 0.0f) {
            state.borderTimer -= delta;
            if (state.borderTimer < 0.0f) state.borderTimer = 0.0f;
        }

        /* ── VISUAL EFFECTS UPDATE ── */
        updateEtincelle(&etincelle, delta);

    }

    /* ── LIBÉRATION ── */
    liberer(&etincelle, &entite, maskSurf, &m);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
