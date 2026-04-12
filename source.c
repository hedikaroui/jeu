#include "minimap.h"
#include <stdio.h>

/* ═══════════════════════════════════════════════
 * FENÊTRE / RENDERER
 * ═══════════════════════════════════════════════ */

SDL_Window* InitFenetre(const char *titre, int largeur, int hauteur)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init : %s\n", SDL_GetError());
        return NULL;
    }
    SDL_Window *w = SDL_CreateWindow(titre,
                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        largeur, hauteur, 0);
    if (!w) fprintf(stderr, "SDL_CreateWindow : %s\n", SDL_GetError());
    return w;
}

SDL_Renderer* InitRenderer(SDL_Window *window)
{
    SDL_Renderer *r = SDL_CreateRenderer(window, -1,
                          SDL_RENDERER_ACCELERATED |
                          SDL_RENDERER_PRESENTVSYNC);
    if (!r) fprintf(stderr, "SDL_CreateRenderer : %s\n", SDL_GetError());
    return r;
}

/* ═══════════════════════════════════════════════
 * CHARGEMENT DES RESSOURCES
 * ═══════════════════════════════════════════════ */

int LoadRessources(Minimap *m, SDL_Renderer *renderer,
                   const char *bgPath, const char *playerPath,
                   int mapX, int mapY, int mapW, int mapH,
                   int pointW, int pointH, int redim)
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
    return 1;
}

/* ═══════════════════════════════════════════════
 * FEATURE 3 : worldToMinimap avec zoom
 * Convertit position monde → position minimap.
 * ═══════════════════════════════════════════════ */
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

/* ═══════════════════════════════════════════════
 * FEATURE 1 : renderBorder
 * Bordure rouge si collision récente, blanche sinon. Épaisseur = 3px.
 * ═══════════════════════════════════════════════ */
void renderBorder(SDL_Renderer *renderer, SDL_Rect minimapPos,
                  float borderTimer)
{
    if (borderTimer > 0.0f)
        SDL_SetRenderDrawColor(renderer, 220, 30,  30,  255);
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

/* ═══════════════════════════════════════════════
 * DANGER ZONES avec zoom
 * ═══════════════════════════════════════════════ */

/* ═══════════════════════════════════════════════
 * afficherMinimap
 * Ordre : clear → bg → entité → joueur →
 *         minimap fond → joueur minimap
 * ═══════════════════════════════════════════════ */
void afficherMinimap(Minimap *m, SDL_Renderer *renderer,
                     GameState *state, Entite *entite)
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 50, 255);
    SDL_RenderClear(renderer);

    SDL_Rect destBG = { 0, 0, WIDTH, HEIGHT };

    /* Background plein écran */
    if (m->background)
        SDL_RenderCopy(renderer, m->background, NULL, &destBG);
    else {
        SDL_SetRenderDrawColor(renderer, 70, 120, 70, 255);
        SDL_RenderFillRect(renderer, &destBG);
    }

    /* Entité secondaire */
    if (entite->texture)
        SDL_RenderCopy(renderer, entite->texture, NULL, &entite->pos);
    else {
        SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
        SDL_RenderFillRect(renderer, &entite->pos);
    }

    /* Joueur plein écran */
    if (m->playerTexture)
        SDL_RenderCopy(renderer, m->playerTexture, NULL, &state->posJoueur);
    else {
        SDL_SetRenderDrawColor(renderer, 50, 100, 220, 255);
        SDL_RenderFillRect(renderer, &state->posJoueur);
    }

    /* Fond minimap */
    if (m->background) {
        SDL_RenderCopy(renderer, m->background, NULL, &m->minimapPosition);
    }

    /* Joueur sur la minimap avec zoom */
    SDL_Rect playerMinimap = worldToMinimap(m, state->posJoueur, state->zoom);
    if (m->playerTexture)
        SDL_RenderCopy(renderer, m->playerTexture, NULL, &playerMinimap);
    else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &playerMinimap);
    }
    m->playerPosition = playerMinimap;

    /* Bordure colorée */
    renderBorder(renderer, m->minimapPosition, state->borderTimer);
}

/* ═══════════════════════════════════════════════
 * VISUAL EFFECT SYSTEM – Etincelle
 * ═══════════════════════════════════════════════ */

int LoadEtincelle(Etincelle *e, SDL_Renderer *renderer,
                  const char *path, int nbFrames,
                  int rows)
{
    SDL_Surface *surf = IMG_Load(path);
    if (!surf) {
        fprintf(stderr, "LoadEtincelle – %s\n", IMG_GetError());
        e->spriteSheet = NULL;
        return 0;
    }
    e->largeurSprite = surf->w;
    e->spriteSheet   = SDL_CreateTextureFromSurface(renderer, surf);
    int frameW = surf->w / nbFrames;
    int frameH = surf->h;
    SDL_FreeSurface(surf);
    e->nbFrames      = nbFrames;
    e->rows          = rows;
    e->direction     = 0;
    e->active        = 0;
    e->frameTimer    = 0.0f;
    e->frameDuration = 0.1f;
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

/* animateEntity (atelier Animation) */
void updateEtincelle(Etincelle *e, float delta)
{
    if (!e->active) return;
    e->frameTimer += delta;
    if (e->frameTimer < e->frameDuration) return;
    e->frameTimer -= e->frameDuration;

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

/* blitEntity (atelier Animation) */
void afficherEtincelle(SDL_Renderer *renderer, Etincelle *e)
{
    if (!e->active || !e->spriteSheet) return;
    SDL_RenderCopy(renderer, e->spriteSheet,
                   &e->posSprite, &e->destRect);
}

void libererEtincelle(Etincelle *e)
{
    if (e->spriteSheet) {
        SDL_DestroyTexture(e->spriteSheet);
        e->spriteSheet = NULL;
    }
}

/* ═══════════════════════════════════════════════
 * LIBÉRATION GLOBALE
 * ═══════════════════════════════════════════════ */

void liberer(Etincelle *etincelle, Entite *entite,
             SDL_Surface *maskSurf, Minimap *m)
{
    libererEtincelle(etincelle);
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

/* ═══════════════════════════════════════════════
 * UPDATE SYSTEMS
 * ═══════════════════════════════════════════════ */

void UpdateGame(SDL_Rect *posJoueur, int gauche, int droite,
                int haut, int bas, float *rotation, Minimap *m)
{
    if (gauche) { posJoueur->x -= VITESSE; *rotation = 180.0f; }
    if (droite) { posJoueur->x += VITESSE; *rotation =   0.0f; }
    if (haut)   { posJoueur->y -= VITESSE; *rotation = 270.0f; }
    if (bas)    { posJoueur->y += VITESSE; *rotation =  90.0f; }

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

void initGameState(GameState *state, SDL_Rect posJoueur, float rotation,
                   DangerZone *zones, int nbZones, float zoom)
{
    state->posJoueur        = posJoueur;
    state->rotation         = rotation;
    state->zones            = zones;
    state->nbZones          = nbZones;
    state->time             = 0;
    state->collisionBBEvent = 0;
    state->collisionPPEvent = 0;
    state->borderTimer      = 0.0f;
    state->zoom             = zoom;
}

/* ═══════════════════════════════════════════════
 * COLLISION SYSTEM – retourne 0/1 uniquement
 * ═══════════════════════════════════════════════ */

void checkCollisionBB(SDL_Rect *posJoueur, SDL_Rect ancienne,
                      Minimap *m, Entite *entite, GameState *state)
{
    int bb = collisionBB(*posJoueur, entite->pos);
    state->collisionBBEvent = bb;
    if (bb) {
        *posJoueur = ancienne;
        m->playerPosition.x = m->minimapPosition.x +
                              (ancienne.x * m->redimensionnement) / 100;
        m->playerPosition.y = m->minimapPosition.y +
                              (ancienne.y * m->redimensionnement) / 100;
    }
}

void checkCollisionPP(SDL_Rect *posJoueur, SDL_Rect ancienne,
                      Minimap *m, SDL_Surface *maskSurf, GameState *state)
{
    int pp = collisionPP(maskSurf, *posJoueur);
    state->collisionPPEvent = pp;
    if (pp) {
        *posJoueur = ancienne;
        m->playerPosition.x = m->minimapPosition.x +
                              (ancienne.x * m->redimensionnement) / 100;
        m->playerPosition.y = m->minimapPosition.y +
                              (ancienne.y * m->redimensionnement) / 100;
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


