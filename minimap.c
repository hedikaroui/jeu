#include "minimap.h"
#include <stdio.h>

int InitSDL_MM(SDL_Window **window, SDL_Renderer **renderer,
            const char *titre, int largeur, int hauteur)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init : %s\n", SDL_GetError());
        return 0;
    }
    *window = SDL_CreateWindow(titre,
                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                  largeur, hauteur, 0);
    if (!*window) {
        fprintf(stderr, "SDL_CreateWindow : %s\n", SDL_GetError());
        return 0;
    }
    *renderer = SDL_CreateRenderer(*window, -1,
                    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) {
        fprintf(stderr, "SDL_CreateRenderer : %s\n", SDL_GetError());
        return 0;
    }
    return 1;
}

int LoadRessources(Minimap *m, GameState *state, SDL_Renderer *renderer,
                   const char *bgPath, const char *playerPath,
                   int mapX, int mapY, int mapW, int mapH,
                   int pointW, int pointH, int redim,
                   SDL_Rect posJoueur, float rotation, float zoom)
{
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) &
          (IMG_INIT_PNG | IMG_INIT_JPG))) {
        fprintf(stderr, "IMG_Init : %s\n", IMG_GetError());
        return 0;
    }
    m->background = IMG_LoadTexture(renderer, bgPath);
    if (!m->background) {
        fprintf(stderr, "IMG_LoadTexture bg : %s\n", IMG_GetError());
        return 0;
    }
    m->playerTexture = IMG_LoadTexture(renderer, playerPath);
    if (!m->playerTexture) {
        fprintf(stderr, "IMG_LoadTexture player : %s\n", IMG_GetError());
        return 0;
    }
    m->minimapPosition   = (SDL_Rect){ mapX, mapY, mapW, mapH };
    m->playerPosition    = (SDL_Rect){ mapX, mapY, pointW, pointH };
    m->redimensionnement = redim;
    m->camera.rect       = (SDL_Rect){ 0, 0, WIDTH, HEIGHT };
    m->running           = 1;

    state->posJoueur        = posJoueur;
    state->rotation         = rotation;
    state->zoom             = zoom;
    state->collisionBBEvent = 0;
    state->collisionPPEvent = 0;
    state->borderTimer      = 0.0f;
    state->gauche           = 0;
    state->droite           = 0;
    state->haut             = 0;
    state->bas              = 0;
    return 1;
}

SDL_Rect worldToMinimap(Minimap *m, SDL_Rect worldPos, float zoom)
{
    float factor = (m->redimensionnement * zoom) / 100.0f;
    SDL_Rect result = {
        m->minimapPosition.x + (int)(worldPos.x * factor),
        m->minimapPosition.y + (int)(worldPos.y * factor),
        (int)(worldPos.w * factor),
        (int)(worldPos.h * factor)
    };
    return result;
}

void renderBorder(SDL_Renderer *renderer, SDL_Rect minimapPos,
                  float borderTimer)
{
    if (borderTimer > 0.0f)
        SDL_SetRenderDrawColor(renderer, 220, 30, 30, 255);
    else
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < 3; i++) {
        SDL_Rect border = {
            minimapPos.x - i, minimapPos.y - i,
            minimapPos.w + i*2, minimapPos.h + i*2
        };
        SDL_RenderDrawRect(renderer, &border);
    }
}

void afficherMinimap(Minimap *m, SDL_Renderer *renderer,
                     GameState *state, Entite *entite)
{
    /* On ne fait PAS de RenderClear ici — le background principal est deja dessine */

    /* Afficher l'entite dans le monde */
    if (entite->texture)
        SDL_RenderCopy(renderer, entite->texture, NULL, &entite->pos);
    else {
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &entite->pos);
    }

    /* Afficher la minimap (fond) : montrer une portion centrée sur le joueur */
    if (m->background) {
        int bw = 0, bh = 0;
        SDL_QueryTexture(m->background, NULL, NULL, &bw, &bh);
        int camW = (m->minimapPosition.w * 100) / m->redimensionnement;
        int camH = (m->minimapPosition.h * 100) / m->redimensionnement;
        if (camW <= 0) camW = m->minimapPosition.w;
        if (camH <= 0) camH = m->minimapPosition.h;
        int cx = state->posJoueur.x + state->posJoueur.w/2 - camW/2;
        int cy = state->posJoueur.y + state->posJoueur.h/2 - camH/2;
        if (cx < 0) cx = 0; if (cy < 0) cy = 0;
        if (bw > camW && cx + camW > bw) cx = bw - camW;
        if (bh > camH && cy + camH > bh) cy = bh - camH;
        m->camera.rect = (SDL_Rect){ cx, cy, camW, camH };
        SDL_RenderCopy(renderer, m->background, &m->camera.rect, &m->minimapPosition);
    }

    /* Afficher le joueur sur la minimap */
    SDL_Rect playerMinimap = worldToMinimap(m, state->posJoueur, state->zoom);
    if (m->playerTexture)
        SDL_RenderCopy(renderer, m->playerTexture, NULL, &playerMinimap);
    else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &playerMinimap);
    }
    m->playerPosition = playerMinimap;
}

int LoadEtincelle(Etincelle *e, SDL_Renderer *renderer,
                  const char *path, int nbFrames, int rows)
{
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        fprintf(stderr, "LoadEtincelle : %s\n", IMG_GetError());
        e->spriteSheet = NULL;
        return 0;
    }
    e->largeurSprite = surf->w;
    e->spriteSheet   = SDL_CreateTextureFromSurface(renderer, surf);
    int frameW = surf->w / nbFrames;
    int frameH = surf->h;
    SDL_FreeSurface(surf);
    e->nbFrames  = nbFrames;
    e->rows      = rows;
    e->direction = 0;
    e->active    = 0;
    e->posSprite = (SDL_Rect){ 0, 0, frameW, frameH };
    e->destRect  = (SDL_Rect){ 0, 0, 20, 20 };
    return 1;
}

void declencherEtincelle(Etincelle *e, SDL_Rect posJoueurMinimap,
                         int direction)
{
    if (!e->spriteSheet) return;
    e->active      = 1;
    e->direction   = direction;
    e->posSprite.x = 0;
    e->destRect.x  = posJoueurMinimap.x - e->destRect.w / 2;
    e->destRect.y  = posJoueurMinimap.y - e->destRect.h / 2;
}

void updateEtincelle(Etincelle *e)
{
    if (!e->active) return;

    e->posSprite.y = e->direction * e->posSprite.h;

    if (e->posSprite.x == e->largeurSprite - e->posSprite.w) {
        e->posSprite.x = 0;
        e->direction++;
        if (e->direction >= e->rows) {
            e->direction = 0;
            e->active    = 0;
        }
    } else {
        e->posSprite.x += e->posSprite.w;
    }
}

void afficherEtincelle(SDL_Renderer *renderer, Etincelle *e)
{
    if (!e->active || !e->spriteSheet) return;
    SDL_RenderCopy(renderer, e->spriteSheet, &e->posSprite, &e->destRect);
}

void Liberation(Etincelle *etincelle, Entite *entite,
                SDL_Surface *maskSurf, Minimap *m)
{
    if (etincelle && etincelle->spriteSheet) {
        SDL_DestroyTexture(etincelle->spriteSheet);
        etincelle->spriteSheet = NULL;
    }
    if (entite && entite->texture) {
        SDL_DestroyTexture(entite->texture);
        entite->texture = NULL;
    }
    if (maskSurf) SDL_FreeSurface(maskSurf);
    if (m && m->background) {
        SDL_DestroyTexture(m->background);
        m->background = NULL;
    }
    if (m && m->playerTexture) {
        SDL_DestroyTexture(m->playerTexture);
        m->playerTexture = NULL;
    }
}

void UpdateGame(SDL_Rect *posJoueur, int gauche, int droite,
                int haut, int bas, float *rotation, Minimap *m)
{
    if (gauche) { posJoueur->x -= VITESSE_MM; *rotation = 180.0f; }
    if (droite) { posJoueur->x += VITESSE_MM; *rotation =   0.0f; }
    if (haut)   { posJoueur->y -= VITESSE_MM; *rotation = 270.0f; }
    if (bas)    { posJoueur->y += VITESSE_MM; *rotation =  90.0f; }

    if (posJoueur->x < 0)                     posJoueur->x = 0;
    if (posJoueur->y < 0)                     posJoueur->y = 0;
    if (posJoueur->x + posJoueur->w > WIDTH)  posJoueur->x = WIDTH  - posJoueur->w;
    if (posJoueur->y + posJoueur->h > HEIGHT) posJoueur->y = HEIGHT - posJoueur->h;

    int absX = posJoueur->x + m->camera.rect.x;
    int absY = posJoueur->y + m->camera.rect.y;
    m->playerPosition.x = m->minimapPosition.x +
                          (absX * m->redimensionnement) / 100;
    m->playerPosition.y = m->minimapPosition.y +
                          (absY * m->redimensionnement) / 100;
}

void Lecture(Minimap *m, GameState *state)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) m->running = 0;
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE: m->running    = 0; break;
                case SDLK_LEFT:   state->gauche = 1; break;
                case SDLK_RIGHT:  state->droite = 1; break;
                case SDLK_UP:     state->haut   = 1; break;
                case SDLK_DOWN:   state->bas    = 1; break;
                case SDLK_z:
                    state->zoom += ZOOM_STEP;
                    if (state->zoom > ZOOM_MAX) state->zoom = ZOOM_MAX;
                    break;
                case SDLK_x:
                    state->zoom -= ZOOM_STEP;
                    if (state->zoom < ZOOM_MIN) state->zoom = ZOOM_MIN;
                    break;
                default: break;
            }
        }
        if (e.type == SDL_KEYUP) {
            switch (e.key.keysym.sym) {
                case SDLK_LEFT:  state->gauche = 0; break;
                case SDLK_RIGHT: state->droite = 0; break;
                case SDLK_UP:    state->haut   = 0; break;
                case SDLK_DOWN:  state->bas    = 0; break;
                default: break;
            }
        }
    }
}

SDL_Color GetPixel(SDL_Surface *surface, int x, int y)
{
    SDL_Color color = { 255, 255, 255, 255 };
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h)
        return color;
    Uint8 *p = (Uint8 *)surface->pixels
             + (y * surface->pitch)
             + (x * surface->format->BytesPerPixel);
    Uint32 pv = 0;
    memcpy(&pv, p, surface->format->BytesPerPixel);
    SDL_GetRGB(pv, surface->format, &color.r, &color.g, &color.b);
    return color;
}

int collisionBB(SDL_Rect pos1, SDL_Rect pos2)
{
    if (pos1.x + pos1.w < pos2.x) return 0;
    if (pos1.x > pos2.x + pos2.w) return 0;
    if (pos1.y + pos1.h < pos2.y) return 0;
    if (pos1.y > pos2.y + pos2.h) return 0;
    return 1;
}

int collisionPP(SDL_Surface *maskSurf, SDL_Rect pos)
{
    if (!maskSurf) return 0;
    int X = pos.x, Y = pos.y, W = pos.w, H = pos.h;
    int points[8][2] = {
        {X,     Y    }, {X+W/2, Y    }, {X+W, Y    },
        {X,     Y+H/2}, {X+W,   Y+H/2},
        {X,     Y+H  }, {X+W/2, Y+H  }, {X+W, Y+H  }
    };
    for (int i = 0; i < 8; i++) {
        SDL_Color c = GetPixel(maskSurf, points[i][0], points[i][1]);
        if (c.r == 0 && c.g == 0 && c.b == 0) return 1;
    }
    return 0;
}
