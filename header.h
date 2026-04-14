#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define WIDTH  800
#define HEIGHT 600

/* ========== PUZZLE ========== */
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

/* Lance le puzzle depuis le menu quiz */
void RunPuzzle(void);

/* ========== QUIZ ========== */
typedef struct {
    char question[300];
    char repA[150];
    char repB[150];
    char repC[150];
    int  bonneReponse;
} Question;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *police;

    SDL_Texture *bg1;
    SDL_Texture *bg2;

    SDL_Texture *btnQuiz;
    SDL_Texture *btnQuizHover;
    SDL_Texture *btnPuzzle;
    SDL_Texture *btnPuzzleHover;

    SDL_Texture *btnA;
    SDL_Texture *btnB;
    SDL_Texture *btnC;

    SDL_Texture *loser;

    Mix_Chunk *beep;
    Mix_Chunk *beep2;
    Mix_Chunk *laugh;
    Mix_Music *musique;

    Question questions[50];
    int      nbQuestions;
    int      dejaPosee[50];
    Question actuelle;

    /* etat: 0=menu  1=quiz  2=puzzle */
    int etat;
    int running;

    int score;
    int feedback;
    int feedbackTimer;

    Uint32 tempsDepart;
    int    tempsRestant;
    int    enigmeFinie;

    int repSurvol;

    SDL_Rect rectQuiz;
    SDL_Rect rectPuzzle;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect rectC;

} enigme;

int  InitSDL         (enigme *e);
int  Initialisation  (enigme *e);
int  Load            (enigme *e);
void Affichage       (enigme *e);
void GererEvenement  (enigme *e, int *running);
void MiseAJour       (enigme *e);
void Liberation      (enigme *e);
void NouvelleQuestion(enigme *e);
void AfficherTexte   (enigme *e, const char *txt, int x, int y, SDL_Color col, int taille);
void AfficherTimeline(enigme *e);

#endif
