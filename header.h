#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WIN_W     900
#define WIN_H     600
#define TIMER_SEC 30
#define NUM_LVL   2

typedef struct {
    SDL_Texture *tex;
    SDL_Rect dst, home;
    int correct, dragging, ox, oy;
} Piece;

typedef struct {
    SDL_Texture *bg;
    SDL_Rect hole;
    Piece pieces[3];
} Level;

typedef struct {
    SDL_Window   *win;
    SDL_Renderer *ren;
    TTF_Font     *font;
    Mix_Music    *music;
    Mix_Chunk    *snd_ok, *snd_wrong;
    SDL_Texture  *background;
    Level         levels[NUM_LVL];
    int           cur;
    Uint32        t0, solved_at;
    int           solved, gameover, wrong_flash;
} Game;

int  game_init   (Game *g);
void game_load   (Game *g);
void reset_pieces(Level *lv);
void game_event  (Game *g, SDL_Event *e);
void game_update (Game *g);
void game_draw   (Game *g);
void game_free   (Game *g);
int  inside      (SDL_Rect *r, int x, int y);
int  overlap     (SDL_Rect *a, SDL_Rect *b);

#endif
