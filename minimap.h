#ifndef MINIMAP_H
#define MINIMAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <math.h>

#define WIDTH   800
#define HEIGHT  600
#define VITESSE 4

/* Zoom config */
#define ZOOM_MIN  0.5f
#define ZOOM_MAX  2.0f
#define ZOOM_STEP 0.1f

typedef struct { SDL_Rect rect; } Camera;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect     pos;
} Entite;

/* Etincelle � atelier Animation */
typedef struct {
    SDL_Texture *spriteSheet;
    SDL_Rect     posSprite;
    SDL_Rect     destRect;
    int          nbFrames;
    int          largeurSprite;
    int          direction;
    int          active;
    int          rows;
    float        frameTimer;
    float        frameDuration;
} Etincelle;

/* DangerZone */
typedef struct {
    SDL_Rect pos;
    float    danger_level;
    int      active;
} DangerZone;

/* GameState � source unique de v�rit� */
typedef struct {
    SDL_Rect    posJoueur;
    float       rotation;
    DangerZone *zones;
    int         nbZones;
    Uint32      time;
    int         collisionBBEvent;
    int         collisionPPEvent;
    float       borderTimer;  /* timer bordure rouge */
    float       zoom;         /* Feature 3 : zoom minimap        */
} GameState;

typedef struct {
    SDL_Texture *background;
    SDL_Texture *playerTexture;
    SDL_Rect     minimapPosition;
    SDL_Rect     playerPosition;
    Camera       camera;
    int          redimensionnement;
    int          running;
} Minimap;

/* Fen�tre / Renderer */
SDL_Window*   InitFenetre  (const char *titre, int largeur, int hauteur);
SDL_Renderer* InitRenderer (SDL_Window *window);

/* Minimap */
int  LoadRessources  (Minimap *m, SDL_Renderer *renderer,
                      const char *bgPath, const char *playerPath,
                      int mapX, int mapY, int mapW, int mapH,
                      int pointW, int pointH, int redim);

/* MinimapSystem */
void afficherMinimap   (Minimap *m, SDL_Renderer *renderer,
                        GameState *state, Entite *entite);
void renderBorder      (SDL_Renderer *renderer, SDL_Rect minimapPos,
                        float borderTimer);
SDL_Rect  worldToMinimap(Minimap *m, SDL_Rect worldPos, float zoom);

/* VisualEffectSystem � Etincelle */
int  LoadEtincelle       (Etincelle *e, SDL_Renderer *renderer,
                          const char *path, int nbFrames,
                          int rows);
void declencherEtincelle (Etincelle *e, SDL_Rect posJoueurMinimap,
                          int direction);
void updateEtincelle     (Etincelle *e, float delta);
void afficherEtincelle   (SDL_Renderer *renderer, Etincelle *e);
void libererEtincelle    (Etincelle *e);

/* CollisionSystem */
void checkCollisionBB (SDL_Rect *posJoueur, SDL_Rect ancienne,
                       Minimap *m, Entite *entite, GameState *state);
void checkCollisionPP (SDL_Rect *posJoueur, SDL_Rect ancienne,
                       Minimap *m, SDL_Surface *maskSurf, GameState *state);
SDL_Color GetPixel    (SDL_Surface *surface, int x, int y);
int       collisionBB (SDL_Rect pos1, SDL_Rect pos2);
int       collisionPP (SDL_Surface *maskSurf, SDL_Rect pos);

/* Update systems */
void UpdateGame  (SDL_Rect *posJoueur, int gauche, int droite,
                  int haut, int bas, float *rotation, Minimap *m);
void initGameState(GameState *state, SDL_Rect posJoueur, float rotation,
                   DangerZone *zones, int nbZones, float zoom);

/* Lib�ration */
void liberer (Etincelle *etincelle, Entite *entite,
              SDL_Surface *maskSurf, Minimap *m);
#endif

