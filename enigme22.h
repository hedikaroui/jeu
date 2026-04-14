#ifndef ENIGME22_H
#define ENIGME22_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* Position et taille du popup sur la fenetre principale 1400x720 */
#define E22_POP_X 300
#define E22_POP_Y  60
#define E22_POP_W 800
#define E22_POP_H 600

/* ═══════════════════════════════════════════
   PUZZLE
═══════════════════════════════════════════ */
#define E22_NUM_LVL   2
#define E22_TIMER_SEC 30
#define E22_WIN_W     900
#define E22_WIN_H     600

typedef struct {
    SDL_Texture *tex;
    SDL_Rect dst, home;
    int correct, dragging, ox, oy;
} E22_Piece;

typedef struct {
    SDL_Texture *bg;
    SDL_Rect hole;
    E22_Piece pieces[3];
} E22_Level;

typedef struct {
    SDL_Window   *win;
    SDL_Renderer *ren;
    TTF_Font     *font;
    Mix_Music    *music;
    Mix_Chunk    *snd_ok, *snd_wrong;
    SDL_Texture  *background;
    E22_Level     levels[E22_NUM_LVL];
    int           cur;
    Uint32        t0, solved_at;
    int           solved, gameover, wrong_flash;
} E22_PuzzleGame;

int  E22_inside      (SDL_Rect *r, int x, int y);
int  E22_overlap     (SDL_Rect *a, SDL_Rect *b);
void E22_reset_pieces(E22_Level *lv);
int  E22_game_init   (E22_PuzzleGame *g);
void E22_game_load   (E22_PuzzleGame *g);
void E22_game_event  (E22_PuzzleGame *g, SDL_Event *e);
void E22_game_update (E22_PuzzleGame *g);
void E22_game_draw   (E22_PuzzleGame *g);
void E22_game_free   (E22_PuzzleGame *g);
void E22_RunPuzzle   (void);

/* ═══════════════════════════════════════════
   QUIZ
═══════════════════════════════════════════ */
typedef struct {
    char question[300];
    char repA[150];
    char repB[150];
    char repC[150];
    int  bonneReponse;
} E22_Question;

typedef struct {
    TTF_Font    *police;

    SDL_Texture *bg1, *bg2;
    SDL_Texture *btnQuiz, *btnQuizHover;
    SDL_Texture *btnPuzzle, *btnPuzzleHover;
    SDL_Texture *btnA, *btnB, *btnC;
    SDL_Texture *loser;

    Mix_Chunk *beep, *beep2, *laugh;
    Mix_Music *musique;

    E22_Question questions[50];
    int          nbQuestions;
    int          dejaPosee[50];
    E22_Question actuelle;

    int etat;        /* 0 = menu  1 = quiz */
    int score;
    int feedback;
    int feedbackTimer;

    Uint32 tempsDepart;
    int    tempsRestant;
    int    enigmeFinie;
    int    repSurvol;

    SDL_Rect rectQuiz, rectPuzzle;
    SDL_Rect rectA, rectB, rectC;
} enigme22;

int  E22_Initialisation  (enigme22 *e);
int  E22_Load            (enigme22 *e, SDL_Renderer *ren);
void E22_Affichage       (enigme22 *e, SDL_Renderer *ren);
void E22_GererEvenement  (enigme22 *e, SDL_Event *ev, int *fermer);
void E22_MiseAJour       (enigme22 *e);
void E22_Liberation      (enigme22 *e);
void E22_NouvelleQuestion(enigme22 *e);
void E22_AfficherTexte   (SDL_Renderer *ren, TTF_Font *police, const char *txt,
                          int x, int y, SDL_Color col, int taille);
void E22_AfficherTimeline(enigme22 *e, SDL_Renderer *ren);

#endif
