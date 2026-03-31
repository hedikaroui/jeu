#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#define WIDTH  1280
#define HEIGHT  720

/* ── States ── */
typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_OPTIONS,
    STATE_SCORES,       /* leaderboard screen from project 2 */
    STATE_CREDITS,
    STATE_QUIT
} GameState;

/* ── Button (project 1 style: normal + hover textures) ── */
typedef struct {
    SDL_Rect      rect;
    int           selected;
    SDL_Texture  *normalTex;
    SDL_Texture  *hoverTex;
} Button;

/* ── Master Game struct ── */
typedef struct {
    /* --- shared / project-1 menu assets --- */
    SDL_Texture  *background;
    TTF_Font     *font;
    SDL_Texture  *titleTexture;
    SDL_Rect      titleRect;
    SDL_Texture  *logoTexture;
    SDL_Rect      logoRect;
    SDL_Texture  *trapTexture;
    SDL_Rect      trapRect;
    Mix_Music    *music;
    Mix_Chunk    *Sound;          /* hover sound */
    Button        buttons[5];    /* Play / Options / Scores / Credits / Quit */

    /* --- project-2 leaderboard assets --- */
    SDL_Texture  *msGreTex;      /* "Scores" button highlighted */
    SDL_Texture  *msRedTex;      /* "Scores" button normal      */
    SDL_Texture  *leaderTex;     /* leaderboard background      */
    Mix_Chunk    *click;         /* click sound                 */
    Button        backBtn;       /* back button on leaderboard  */

    /* --- game state assets --- */
    Button        gameButtons[5]; /* j1, o1, s1, c1, h1 - all lead to scores */

    /* --- search box (project 2) --- */
    SDL_Rect      searchBox;
    int           inputActive;
    char          inputText[64];

    /* --- state machine --- */
    GameState     currentState;
    int           running;
} Game;

/* ── Function prototypes ── */
/* Asset helpers */
SDL_Texture* ChargerTexture(SDL_Renderer *renderer, const char *fichier);
Mix_Music*   ChargerMusique(const char *fichier);
Mix_Chunk*   ChargerSon    (const char *fichier);

/* Lifecycle */
int  Initialisation(Game *game, SDL_Window **window, SDL_Renderer **renderer);
void Liberation    (Game *game, SDL_Window  *window, SDL_Renderer  *renderer);

/* Per-state input + render */
void Menu_LectureEntree       (Game *game);
void Menu_Affichage           (Game *game, SDL_Renderer *renderer);
void Leaderboard_LectureEntree(Game *game);
void Leaderboard_Affichage    (Game *game, SDL_Renderer *renderer);
void Jeu_LectureEntree        (Game *game);
void Jeu_Affichage            (Game *game, SDL_Renderer *renderer);
void HandleGameButtonClick    (Game *game);

#endif /* HEADER_H */
