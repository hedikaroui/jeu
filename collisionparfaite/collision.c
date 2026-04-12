#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include "collision.h"


int InitSDLWindowRenderer(SDL_Window **window, SDL_Renderer **renderer,
                          const char *titre, int w, int h)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Erreur SDL_Init : %s\n", SDL_GetError());
        return 0;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        fprintf(stderr, "Erreur IMG_Init : %s\n", IMG_GetError());
        SDL_Quit();
        return 0;
    }

    *window = SDL_CreateWindow(titre,
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               w, h, 0);
    if (*window == NULL) {
        fprintf(stderr, "Erreur SDL_CreateWindow : %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    *renderer = SDL_CreateRenderer(*window, -1,
                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (*renderer == NULL) {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }

    return 1;
}


void InitGameBB(GameBB *gb, SDL_Renderer *renderer) {
    gb->running       = 1;
    gb->nbPlateformes = 3;

    SDL_Surface *surf = IMG_Load("assets/background.bmp");
    if (surf != NULL) {
        gb->bgTexture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        gb->bgTexture = NULL;
    }

    gb->joueur.sprite    = NULL;
    gb->joueur.pos.x     = WIDTH / 2;
    gb->joueur.pos.y     = HEIGHT / 2;
    gb->joueur.pos.w     = 40;
    gb->joueur.pos.h     = 50;
    gb->joueur.vitesse   = 4;
    gb->joueur.direction = -1;

    gb->plateformes[0] = (Plateforme){ {100, 300, 200, 20}, 0, 1 };
    gb->plateformes[1] = (Plateforme){ {450, 220, 180, 20}, 0, 1 };
    gb->plateformes[2] = (Plateforme){ {580, 380, 150, 20}, 1, 1 };
}


void InitGamePixel(GamePixel *gp, SDL_Renderer *renderer) {
    gp->running  = 1;

    SDL_Surface *surf = IMG_Load("assets/background.bmp");
    if (surf != NULL) {
        gp->bgTexture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        gp->bgTexture = NULL;
    }

    gp->maskSurf = SDL_LoadBMP("assets/backgroundmasq.bmp");
    if (gp->maskSurf == NULL) {
        fprintf(stderr, "[PP] Masque introuvable – collision désactivée\n");
    } else {
        printf("[PP] Masque chargé : %d x %d\n", gp->maskSurf->w, gp->maskSurf->h);
    }

    gp->joueur.sprite    = NULL;
    gp->joueur.pos.x     = WIDTH / 2;
    gp->joueur.pos.y     = HEIGHT / 2;
    gp->joueur.pos.w     = 40;
    gp->joueur.pos.h     = 50;
    gp->joueur.vitesse   = 4;
    gp->joueur.direction = -1;
}


void afficherJeu(GameBB *gb, GamePixel *gp, SDL_Renderer *renderer, int mode, int collision) {
    SDL_Rect destBG = {0, 0, WIDTH, HEIGHT};

    if (mode == MODE_BB) {
        if (gb->bgTexture) SDL_RenderCopy(renderer, gb->bgTexture, NULL, &destBG);
        else {
            SDL_SetRenderDrawColor(renderer, 40, 80, 40, 255);
            SDL_RenderFillRect(renderer, &destBG);
        }

        for (int i = 0; i < gb->nbPlateformes; i++) {
            if (gb->plateformes[i].active == 0) continue;
            if (gb->plateformes[i].destructible == 1)
                SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
            else
                SDL_SetRenderDrawColor(renderer, 50, 180, 50, 255);
            SDL_RenderFillRect(renderer, &gb->plateformes[i].pos);
        }

        SDL_SetRenderDrawColor(renderer, 50, 100, 220, 255);
        SDL_RenderFillRect(renderer, &gb->joueur.pos);

    } else {
        if (gp->bgTexture) SDL_RenderCopy(renderer, gp->bgTexture, NULL, &destBG);
        else {
            SDL_SetRenderDrawColor(renderer, 40, 80, 40, 255);
            SDL_RenderFillRect(renderer, &destBG);
        }

        if (collision == 1) SDL_SetRenderDrawColor(renderer, 220, 50, 50, 255);
        else SDL_SetRenderDrawColor(renderer, 50, 100, 220, 255);

        SDL_RenderFillRect(renderer, &gp->joueur.pos);
    }

    SDL_RenderPresent(renderer);
}
void updateJeu(GameBB *gb, GamePixel *gp, int mode, int *collision,
               int gauche, int droite, int haut, int bas) {

    if (mode == MODE_BB) {
        SDL_Rect ancienne = gb->joueur.pos;
        if (gauche) gb->joueur.pos.x -= gb->joueur.vitesse;
        if (droite) gb->joueur.pos.x += gb->joueur.vitesse;
        if (haut)   gb->joueur.pos.y -= gb->joueur.vitesse;
        if (bas)    gb->joueur.pos.y += gb->joueur.vitesse;              

        if (gb->joueur.pos.x < 0) gb->joueur.pos.x = 0;
        if (gb->joueur.pos.y < 0) gb->joueur.pos.y = 0;
        if (gb->joueur.pos.x + gb->joueur.pos.w > WIDTH)  gb->joueur.pos.x = WIDTH - gb->joueur.pos.w;
        if (gb->joueur.pos.y + gb->joueur.pos.h > HEIGHT) gb->joueur.pos.y = HEIGHT - gb->joueur.pos.h;

        for (int i = 0; i < gb->nbPlateformes; i++) {
            if (collisionBoundingBox(&gb->joueur, &gb->plateformes[i])) {
                if (gb->plateformes[i].destructible == 1) {
                    gb->plateformes[i].active = 0;
                } else {
                    gb->joueur.pos = ancienne;
                }
            }
        }

    } else {
        SDL_Rect ancienne = gp->joueur.pos;
        if (gauche) gp->joueur.pos.x -= gp->joueur.vitesse;
        if (droite) gp->joueur.pos.x += gp->joueur.vitesse;
        if (haut)   gp->joueur.pos.y -= gp->joueur.vitesse;
        if (bas)    gp->joueur.pos.y += gp->joueur.vitesse;

        if (gp->joueur.pos.x < 0) gp->joueur.pos.x = 0;
        if (gp->joueur.pos.y < 0) gp->joueur.pos.y = 0;
        if (gp->joueur.pos.x + gp->joueur.pos.w > WIDTH)  gp->joueur.pos.x = WIDTH - gp->joueur.pos.w;
        if (gp->joueur.pos.y + gp->joueur.pos.h > HEIGHT) gp->joueur.pos.y = HEIGHT - gp->joueur.pos.h;

        *collision = collisionPixelPerfect(gp);
        if (*collision == 1) {
            gp->joueur.pos = ancienne;
        }
    }
}
void Cleanup(GameBB *gb, GamePixel *gp, SDL_Renderer *renderer, SDL_Window *window) {
    if (gb->bgTexture != NULL) SDL_DestroyTexture(gb->bgTexture);
    if (gp->bgTexture != NULL) SDL_DestroyTexture(gp->bgTexture);
    if (gp->maskSurf != NULL) SDL_FreeSurface(gp->maskSurf);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}


SDL_Color GetPixel(SDL_Surface *surface, int x, int y) {
    SDL_Color color = {255, 255, 255, 255}; 

    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) { //retourne la couleur par défaut si les positions sont en surface 
        return color;
    }

    Uint8 *pPosition = (Uint8 *)surface->pixels //calcule l'adresse en memoire du pixel 
                       + (y * surface->pitch)
                       + (x * surface->format->BytesPerPixel);

    Uint32 pixel_value = 0; //lecture de la valeur brute 
    memcpy(&pixel_value, pPosition, surface->format->BytesPerPixel);

    SDL_GetRGB(pixel_value, surface->format, &color.r, &color.g, &color.b); //conversion en RGB 

    return color;
}


int collisionPixelPerfect(GamePixel *g) {
    if (g->maskSurf == NULL) return 0;

    SDL_Rect r = g->joueur.pos;
    SDL_Surface *mask = g->maskSurf;

    int X = r.x;
    int Y = r.y;
    int W = r.w;
    int H = r.h;

    int points[8][2] = {
        {X, Y},             
        {X + W/2, Y},       
        {X + W, Y},         
        {X, Y + H/2},       
        {X, Y + H},         
        {X + W/2, Y + H},   
        {X + W, Y + H},    
        {X + W, Y + H/2}    
    };

    for (int i = 0; i < 8; i++) {
        SDL_Color c = GetPixel(mask, points[i][0], points[i][1]);
        if (c.r == 0 && c.g == 0 && c.b == 0) {
            printf("[PP] Collision détectée au point %d (%d,%d)\n",
                   i+1, points[i][0], points[i][1]);
            return 1;
        }
    }

    return 0;
}


int collisionBoundingBox(Joueur *j, Plateforme *p) {
    if (p->active == 0) return 0;

    if (j->pos.x + j->pos.w < p->pos.x) return 0;             
    if (j->pos.x > p->pos.x + p->pos.w) return 0;           
    if (j->pos.y + j->pos.h < p->pos.y) return 0;          
    if (j->pos.y > p->pos.y + p->pos.h) return 0;             

    return 1; 
}


