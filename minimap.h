#ifndef MINIMAP_H
#define MINIMAP_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <math.h>
#define WIDTH   800
#define HEIGHT  600
#define VITESSE_MM 8
#define ZOOM_MIN  0.5f
#define ZOOM_MAX  2.0f
#define ZOOM_STEP 0.1f
typedef struct { SDL_Rect rect; } Camera;
typedef struct {
    SDL_Texture *texture;
    SDL_Rect     pos;
} Entite;
typedef struct {
    SDL_Texture *spriteSheet;
    SDL_Rect     posSprite;
    SDL_Rect     destRect;
    int          nbFrames;
    int          largeurSprite;
    int          rows;
    int          direction;
    int          active;
} Etincelle;
typedef struct {
    SDL_Rect posJoueur;
    float    rotation;
    int      collisionBBEvent;
    int      collisionPPEvent;
    float    borderTimer;
    float    zoom;
    int      gauche, droite, haut, bas;
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
int InitSDL_MM(SDL_Window **window, SDL_Renderer **renderer,
            const char *titre, int largeur, int hauteur);
int LoadRessources(Minimap *m, GameState *state, SDL_Renderer *renderer,
                   const char *bgPath, const char *playerPath,
                   int mapX, int mapY, int mapW, int mapH,
                   int pointW, int pointH, int redim,
                   SDL_Rect posJoueur, float rotation, float zoom);
SDL_Rect worldToMinimap(Minimap *m, SDL_Rect worldPos, float zoom);
void renderBorder(SDL_Renderer *renderer, SDL_Rect minimapPos,
                  float borderTimer);
void afficherMinimap(Minimap *m, SDL_Renderer *renderer,
                     GameState *state, Entite *entite);
int  LoadEtincelle(Etincelle *e, SDL_Renderer *renderer,
                   const char *path, int nbFrames, int rows);
void declencherEtincelle(Etincelle *e, SDL_Rect posJoueurMinimap,
                         int direction);
void updateEtincelle(Etincelle *e);
void afficherEtincelle(SDL_Renderer *renderer, Etincelle *e);
SDL_Color GetPixel(SDL_Surface *surface, int x, int y);
int       collisionBB(SDL_Rect pos1, SDL_Rect pos2);
int       collisionPP(SDL_Surface *maskSurf, SDL_Rect pos);
void UpdateGame(SDL_Rect *posJoueur, int gauche, int droite,
                int haut, int bas, float *rotation, Minimap *m);
void Lecture(Minimap *m, GameState *state);
void Liberation(Etincelle *etincelle, Entite *entite,
                SDL_Surface *maskSurf, Minimap *m);
#endif
