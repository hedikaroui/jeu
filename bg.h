#ifndef BG_H
#define BG_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH     1400
#define SCREEN_HEIGHT    720
#define LEVEL_WIDTH      2000
#define LEVEL_HEIGHT     562
#define CAM_SPEED        0.12f
#define VITESSE          5

#define PLATFORM_FIXED        0
#define PLATFORM_MOVING       1
#define PLATFORM_DESTRUCTIBLE 2

#define STATE_MENU       0
#define STATE_NAME_INPUT 1
#define STATE_GAME       2

typedef struct { SDL_Rect rect; int type; int active; } Platform;

typedef struct {
    SDL_Rect      rect;
    int           lives, score, enigmasSolved;
    SDL_Texture  *sprite;
    char          name[64];
} Player;

typedef struct {
    SDL_Texture *image;
    SDL_Rect     camera1, camera2, posEcran1, posEcran2;
    int          direction;
    float        camX1, camX2;
} Background;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font, *fontLarge;
    Background    background;
    Platform     *platforms;
    int           platformCount;
    Player        player1, player2;
    Uint32        startTime;
    int           splitScreen, showGuide, inputTarget, inputLen;
    int           state;
    char          inputBuffer[64];
    SDL_Texture  *nameBg;
    SDL_Rect      guideBtnClose;
} Game;

/* init / cleanup */
int          InitSDL(Game *g, const char *title, int w, int h);
void         CleanupSDL(Game *g);
int          LoadBackground(Game *g, const char *path);
int          LoadNameBackground(Game *g, const char *path);
void         InitBackground(Game *g, int w, int h);
void         InitPlatforms(Game *g, int level);
int          InitPlayer(Player *p, SDL_Renderer *r, const char *path, int x, int y);
Uint32       GetElapsedTime(Game *g);
int          PointInRect(int x, int y, SDL_Rect r);

/* render helpers */
SDL_Texture *LoadTex(Game *g, const char *path);
void         DrawFilledRect(Game *g, SDL_Rect r, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
void         DrawBorderRect(Game *g, SDL_Rect r, Uint8 R, Uint8 G, Uint8 B, Uint8 A);
void         RenderText(Game *g, TTF_Font *f, const char *txt, SDL_Color c, int x, int y);
void         RenderTextW(Game *g, TTF_Font *f, const char *txt, SDL_Color c, int x, int y, int wrap);
void         RenderTextCentered(Game *g, TTF_Font *f, const char *txt, SDL_Color c, SDL_Rect box);
void         ClampPlayer(Player *p, int lw, int lh);
void         SmoothCam(float *camX, int playerX, int camW, int levelW, int offset);
void         GoToNameInput(Game *g, int split);
void         RenderPlayerRect(Game *g, Player *p, SDL_Rect *cam);
void         RenderPlatformsOnCam(Game *g, SDL_Rect *cam);

/* states */
void         RenderMenu(Game *g);
void         RenderNameInput(Game *g);
void         RenderGuideOverlay(Game *g);
void         RenderTime(Game *g, SDL_Rect *pos);
void         RenderGameSingle(Game *g, int keys[]);
void         RenderGameSplit(Game *g, int keys[]);

/* events */
void         HandleMenuClick(Game *g, int mx, int my);
void         HandleNameInput(Game *g, SDL_Event *e);
void         HandleGuideClick(Game *g, int mx, int my);

/* update */
void         UpdatePlayersSingle(Game *g, int keys[], int lw, int lh);
void         UpdatePlayersSplit(Game *g, int keys[], int lw, int lh);

#endif

