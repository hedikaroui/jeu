#ifndef COLLISION_H
#define COLLISION_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define WIDTH  800
#define HEIGHT 480

#define MODE_BB    0
#define MODE_PIXEL 1

typedef struct {
    SDL_Texture *sprite;
    SDL_Rect     pos;
    int          vitesse;
    int          direction;
} Joueur;

typedef struct {
    SDL_Rect pos;
    int      destructible;
    int      active;
} Plateforme;

typedef struct {
    SDL_Surface  *maskSurf;
    SDL_Texture  *bgTexture;
    Joueur        joueur;
    int           running;
} GamePixel;

typedef struct {
    SDL_Texture  *bgTexture;
    Joueur        joueur;
    Plateforme    plateformes[3];
    int           nbPlateformes;
    int           running;
} GameBB;


int InitSDLWindowRenderer(SDL_Window **window, SDL_Renderer **renderer,
                          const char *titre, int w, int h);

void InitGameBB(GameBB *gb, SDL_Renderer *renderer);
void InitGamePixel(GamePixel *gp, SDL_Renderer *renderer);

void afficherJeu(GameBB *gb, GamePixel *gp, SDL_Renderer *renderer, int mode, int collision);

void updateJeu(GameBB *gb, GamePixel *gp, int mode, int *collision,
               int gauche, int droite, int haut, int bas);
int collisionPixelPerfect(GamePixel *g) ; 
int collisionBoundingBox(Joueur *j, Plateforme *p) ; 


void Cleanup(GameBB *gb, GamePixel *gp, SDL_Renderer *renderer, SDL_Window *window) ; 
#endif
