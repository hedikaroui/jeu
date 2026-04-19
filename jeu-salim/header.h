#ifndef GAME_HEADER_H
#define GAME_HEADER_H

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
    SDL_Rect     rect;
    int          lives, score, enigmasSolved;
    SDL_Texture *sprite;
    char         name[64];
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
    int           state, running;
    char          inputBuffer[64];
    SDL_Texture  *nameBg;
    SDL_Rect      guideBtnClose;
    SDL_Rect      modeBtn1, modeBtn2, modeBtn3;
} Game;

int  InitGame(Game *g, const char *title, int w, int h);
int  LoadRessources(Game *g, const char *bgPath, const char *nameBgPath);
void HandleEvents(Game *g, int keys[]);
void Update(Game *g, int keys[]);
void Render(Game *g, int keys[]);
void Cleanup(Game *g);

#endif
