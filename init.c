#include "header.h"
#include <stdio.h>

int Initialisation(Game *game, SDL_Window **window, SDL_Renderer **renderer)
{
    /* --- SDL subsystems --- */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    /* --- Window & Renderer --- */
    *window = SDL_CreateWindow("Jeu Principal",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!*window) { fprintf(stderr, "Fenetre: %s\n", SDL_GetError()); return 0; }

    *renderer = SDL_CreateRenderer(*window, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) { fprintf(stderr, "Renderer: %s\n", SDL_GetError()); return 0; }

    /* ── Project-1 menu assets ── */
    game->background    = ChargerTexture(*renderer, "background.png");
    game->logoTexture   = ChargerTexture(*renderer, "logo.png");
    game->logoRect      = (SDL_Rect){250, 50, 300, 150};
    game->titleTexture  = ChargerTexture(*renderer, "logo.png");
    game->titleRect     = (SDL_Rect){150, 10, 500, 100};
    game->trapTexture   = ChargerTexture(*renderer, "background.png");
    game->trapRect      = (SDL_Rect){0, 0, WIDTH, HEIGHT};

    game->music = ChargerMusique("jingle.mp3");
    if (game->music) Mix_PlayMusic(game->music, -1);
    game->Sound = ChargerSon("magic.wav");

    /* Menu buttons: Play / Options / Scores / Credits / Quit */
    const char *normalFiles[] = {"m1.png","o1.png","s1.png","j1.png","h1.png"};
    const char *hoverFiles[]  = {"m2.png","o2.png","s2.png","j2.png","h2.png"};
    for (int i = 0; i < 5; i++) {
        game->buttons[i].normalTex = ChargerTexture(*renderer, normalFiles[i]);
        game->buttons[i].hoverTex  = ChargerTexture(*renderer, hoverFiles[i]);
        game->buttons[i].rect      = (SDL_Rect){150 + i*120, 250 + i*50, 100, 40};
        game->buttons[i].selected  = 0;
    }

    /* ── Project-2 leaderboard assets ── */
    game->msGreTex  = ChargerTexture(*renderer, "MSgre.png");
    game->msRedTex  = ChargerTexture(*renderer, "MSred.png");
    game->leaderTex = ChargerTexture(*renderer, "LB.jpg");
    game->click     = ChargerSon("click.mp3");

    /* Back button (top-right of leaderboard screen) */
    game->backBtn.rect     = (SDL_Rect){900, 10, 300, 120};
    game->backBtn.selected = 0;

    /* Game buttons - all click sequences lead to scores */
    const char *gameFiles[] = {"j1.png", "o1.png", "s1.png", "c1.png", "h1.png"};
    const char *gameHover[] = {"j2.png", "o2.png", "s2.png", "c2.png", "h2.png"};
    for (int i = 0; i < 5; i++) {
        game->gameButtons[i].normalTex = ChargerTexture(*renderer, gameFiles[i]);
        game->gameButtons[i].hoverTex  = ChargerTexture(*renderer, gameHover[i]);
        game->gameButtons[i].rect      = (SDL_Rect){150 + i*200, 300, 100, 40};
        game->gameButtons[i].selected  = 0;
    }

    /* Search box */
    game->searchBox    = (SDL_Rect){560, 180, 240, 40};
    game->inputActive  = 0;
    game->inputText[0] = '\0';

    /* Font (used for search box text) */
    game->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!game->font)
        fprintf(stderr, "Erreur police: %s\n", TTF_GetError());

    /* ── Initial state ── */
    game->currentState = STATE_MENU;
    game->running      = 1;
    return 1;
}

void Liberation(Game *game, SDL_Window *window, SDL_Renderer *renderer)
{
    /* Project-1 assets */
    if (game->music)        Mix_FreeMusic(game->music);
    if (game->Sound)        Mix_FreeChunk(game->Sound);
    if (game->font)         TTF_CloseFont(game->font);
    if (game->background)   SDL_DestroyTexture(game->background);
    if (game->logoTexture)  SDL_DestroyTexture(game->logoTexture);
    if (game->titleTexture) SDL_DestroyTexture(game->titleTexture);
    if (game->trapTexture)  SDL_DestroyTexture(game->trapTexture);
    for (int i = 0; i < 5; i++) {
        if (game->buttons[i].normalTex) SDL_DestroyTexture(game->buttons[i].normalTex);
        if (game->buttons[i].hoverTex)  SDL_DestroyTexture(game->buttons[i].hoverTex);
    }

    /* Project-2 assets */
    if (game->msGreTex)  SDL_DestroyTexture(game->msGreTex);
    if (game->msRedTex)  SDL_DestroyTexture(game->msRedTex);
    if (game->leaderTex) SDL_DestroyTexture(game->leaderTex);
    if (game->click)     Mix_FreeChunk(game->click);

    /* Game state assets */
    for (int i = 0; i < 5; i++) {
        if (game->gameButtons[i].normalTex) SDL_DestroyTexture(game->gameButtons[i].normalTex);
        if (game->gameButtons[i].hoverTex)  SDL_DestroyTexture(game->gameButtons[i].hoverTex);
    }

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}
