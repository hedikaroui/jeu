#include "game.h"
#include "assets_catalog.h"
#include <stdio.h>
#include <string.h>

/* Implemented in start_play_state.c */
void StartPlay_Cleanup(void);

int Initialisation(Game *game, SDL_Window **window, SDL_Renderer **renderer) {
    memset(game, 0, sizeof(*game));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    *window = SDL_CreateWindow("Menu Principal",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!*window) {
        printf("Erreur fenetre: %s\n", SDL_GetError());
        return 0;
    }

    *renderer = SDL_CreateRenderer(*window, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) {
        printf("Erreur renderer: %s\n", SDL_GetError());
        return 0;
    }

    Menu_Preparer(game, *renderer);

    game->music = Mix_LoadMUS(GAME_ASSETS.songs.menu_music);
    game->Sound = Mix_LoadWAV(SOUND_HOVER);

    game->saveBg = NULL;
    game->titleSurface = NULL;
    game->titleTexSave = NULL;
    game->saveMusic = NULL;
    game->saveSound = NULL;
    game->saveEtat = 0;
    game->clic_bouton = -1;
    for (int i = 0; i < 4; i++) {
        game->saveButtons[i].texture = NULL;
        game->saveButtons[i].textureCliq = NULL;
        game->saveButtons[i].selected = 0;
    }

    game->ps_loaded = 0;
    game->player1_name[0] = '\0';
    game->player2_name[0] = '\0';
    game->player1Tex = NULL;
    game->player2Tex = NULL;
    game->gameBgTex = NULL;
    game->psJ1Tex = NULL;
    game->psJ2Tex = NULL;
    game->psKeyboardTex = NULL;
    game->psKeyboardHoverTex = NULL;
    game->psManetteTex = NULL;
    game->psManetteHoverTex = NULL;
    game->psSourisTex = NULL;
    game->psSourisHoverTex = NULL;
    game->psScoreBtnTex = NULL;
    game->psMonoBtnTex = NULL;
    game->psMonoBtnHoverTex = NULL;
    game->psMultiBtnTex = NULL;
    game->psMultiBtnHoverTex = NULL;
    game->psNamePlayer1Tex = NULL;
    game->psNamePlayer2Tex = NULL;
    game->psHelpIconTex = NULL;
    game->psHelpIconHoverTex = NULL;
    game->psHelpButtonTex = NULL;
    game->player_mode = 2;
    memset(&game->ps_bg, 0, sizeof(game->ps_bg));

    game->msGreTex = IMG_LoadTexture(*renderer, SCORE_BACK_HOVER);
    game->msRedTex = IMG_LoadTexture(*renderer, SCORE_BACK_NORMAL);
    game->leaderTex = IMG_LoadTexture(*renderer, GAME_ASSETS.backgrounds.leaderboard);
    game->click = Mix_LoadWAV(SOUND_CLICK);
    game->scoreJ1Tex = IMG_LoadTexture(*renderer, SCORE_BUTTON_NORMAL);
    game->scoreJ2Tex = IMG_LoadTexture(*renderer, SCORE_BUTTON_HOVER);
    game->startHeartTex = NULL;
    game->startTextTex = NULL;
    game->startTextRect = (SDL_Rect){0, 0, 0, 0};
    game->startPlayLoaded = 0;

    game->backBtn.rect = (SDL_Rect){WIDTH - 220, 20, 200, 70};
    game->backBtn.selected = 0;
    game->backBtn.normalTex = game->msRedTex;
    game->backBtn.hoverTex = game->msGreTex;

    game->searchBox = (SDL_Rect){(WIDTH - 400) / 2, 140, 400, 50};
    game->inputActive = 0;
    game->inputText[0] = '\0';

    game->optionsBg = NULL;
    game->optionsTitle = NULL;
    game->volumePlusBtn = NULL;
    game->volumeMinusBtn = NULL;
    game->volumeMuteBtn = NULL;
    game->volumePlusHoverBtn = NULL;
    game->volumeMinusHoverBtn = NULL;
    game->volumeMuteHoverBtn = NULL;
    game->fullscreenBtn = NULL;
    game->normalscreenBtn = NULL;
    game->optionsClick = NULL;
    game->optionsFullscreen = 0;
    game->optionsLoaded = 0;

    game->menuGiftTex = IMG_LoadTexture(*renderer, GAME_ASSETS.backgrounds.gift);
    game->gamesImg = NULL;
    game->gamesLoaded = 0;
    memset(&game->gamesRect, 0, sizeof(game->gamesRect));
    game->quizBg1 = NULL;
    game->quizBg2 = NULL;
    game->quizBtnA = NULL;
    game->quizBtnB = NULL;
    game->quizBtnC = NULL;
    game->quizFont = NULL;
    game->quizMusic = NULL;
    game->quizBeep = NULL;
    game->quizBeep2 = NULL;
    game->quizLaugh = NULL;

    game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 32);
    if (!game->font)
        game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 32);

    if (game->music) Mix_PlayMusic(game->music, -1);

    game->window = *window;
    game->volume = Mix_VolumeMusic(-1);
    game->fullscreen = 0;
    game->currentState = STATE_MENU;
    game->running = 1;
    return 1;
}

void Liberation(Game *game, SDL_Window *window, SDL_Renderer *renderer) {
    if (game->music) Mix_FreeMusic(game->music);
    if (game->Sound) Mix_FreeChunk(game->Sound);
    if (game->font) TTF_CloseFont(game->font);
    if (game->background) SDL_DestroyTexture(game->background);
    if (game->logoTexture) SDL_DestroyTexture(game->logoTexture);
    if (game->titleTexture) SDL_DestroyTexture(game->titleTexture);
    if (game->trapTexture) SDL_DestroyTexture(game->trapTexture);
    for (int i = 0; i < 5; i++) {
        if (game->buttons[i].normalTex) SDL_DestroyTexture(game->buttons[i].normalTex);
        if (game->buttons[i].hoverTex) SDL_DestroyTexture(game->buttons[i].hoverTex);
    }

    if (game->saveMusic) Mix_FreeMusic(game->saveMusic);
    if (game->saveSound) Mix_FreeChunk(game->saveSound);
    if (game->titleSurface) SDL_FreeSurface(game->titleSurface);
    if (game->titleTexSave) SDL_DestroyTexture(game->titleTexSave);
    if (game->saveBg) SDL_DestroyTexture(game->saveBg);
    for (int i = 0; i < 4; i++) {
        if (game->saveButtons[i].texture) SDL_DestroyTexture(game->saveButtons[i].texture);
        if (game->saveButtons[i].textureCliq) SDL_DestroyTexture(game->saveButtons[i].textureCliq);
    }

    if (game->ps_bg.texture) SDL_DestroyTexture(game->ps_bg.texture);
    if (game->ps_bg.music) {
        Mix_HaltMusic();
        Mix_FreeMusic(game->ps_bg.music);
    }
    if (game->player1Tex) SDL_DestroyTexture(game->player1Tex);
    if (game->player2Tex) SDL_DestroyTexture(game->player2Tex);
    if (game->gameBgTex) SDL_DestroyTexture(game->gameBgTex);
    if (game->psJ1Tex) SDL_DestroyTexture(game->psJ1Tex);
    if (game->psJ2Tex) SDL_DestroyTexture(game->psJ2Tex);
    if (game->psKeyboardTex) SDL_DestroyTexture(game->psKeyboardTex);
    if (game->psKeyboardHoverTex) SDL_DestroyTexture(game->psKeyboardHoverTex);
    if (game->psManetteTex) SDL_DestroyTexture(game->psManetteTex);
    if (game->psManetteHoverTex) SDL_DestroyTexture(game->psManetteHoverTex);
    if (game->psSourisTex) SDL_DestroyTexture(game->psSourisTex);
    if (game->psSourisHoverTex) SDL_DestroyTexture(game->psSourisHoverTex);
    if (game->psScoreBtnTex) SDL_DestroyTexture(game->psScoreBtnTex);
    if (game->psMonoBtnTex) SDL_DestroyTexture(game->psMonoBtnTex);
    if (game->psMonoBtnHoverTex) SDL_DestroyTexture(game->psMonoBtnHoverTex);
    if (game->psMultiBtnTex) SDL_DestroyTexture(game->psMultiBtnTex);
    if (game->psMultiBtnHoverTex) SDL_DestroyTexture(game->psMultiBtnHoverTex);
    if (game->psNamePlayer1Tex) SDL_DestroyTexture(game->psNamePlayer1Tex);
    if (game->psNamePlayer2Tex) SDL_DestroyTexture(game->psNamePlayer2Tex);
    if (game->psHelpIconTex) SDL_DestroyTexture(game->psHelpIconTex);
    if (game->psHelpIconHoverTex) SDL_DestroyTexture(game->psHelpIconHoverTex);
    if (game->psHelpButtonTex) SDL_DestroyTexture(game->psHelpButtonTex);

    if (game->msGreTex) SDL_DestroyTexture(game->msGreTex);
    if (game->msRedTex) SDL_DestroyTexture(game->msRedTex);
    if (game->leaderTex) SDL_DestroyTexture(game->leaderTex);
    if (game->scoreJ1Tex) SDL_DestroyTexture(game->scoreJ1Tex);
    if (game->scoreJ2Tex) SDL_DestroyTexture(game->scoreJ2Tex);
    if (game->startHeartTex) SDL_DestroyTexture(game->startHeartTex);
    if (game->startTextTex) SDL_DestroyTexture(game->startTextTex);
    if (game->click) Mix_FreeChunk(game->click);

    if (game->optionsBg) SDL_DestroyTexture(game->optionsBg);
    if (game->optionsTitle) SDL_DestroyTexture(game->optionsTitle);
    if (game->volumePlusBtn) SDL_DestroyTexture(game->volumePlusBtn);
    if (game->volumeMinusBtn) SDL_DestroyTexture(game->volumeMinusBtn);
    if (game->volumeMuteBtn) SDL_DestroyTexture(game->volumeMuteBtn);
    if (game->volumePlusHoverBtn) SDL_DestroyTexture(game->volumePlusHoverBtn);
    if (game->volumeMinusHoverBtn) SDL_DestroyTexture(game->volumeMinusHoverBtn);
    if (game->volumeMuteHoverBtn) SDL_DestroyTexture(game->volumeMuteHoverBtn);
    if (game->fullscreenBtn) SDL_DestroyTexture(game->fullscreenBtn);
    if (game->normalscreenBtn) SDL_DestroyTexture(game->normalscreenBtn);
    if (game->optionsClick) Mix_FreeChunk(game->optionsClick);

    if (game->menuGiftTex) SDL_DestroyTexture(game->menuGiftTex);
    if (game->gamesImg) SDL_DestroyTexture(game->gamesImg);
    if (game->quizBg1) SDL_DestroyTexture(game->quizBg1);
    if (game->quizBg2) SDL_DestroyTexture(game->quizBg2);
    if (game->quizBtnA) SDL_DestroyTexture(game->quizBtnA);
    if (game->quizBtnB) SDL_DestroyTexture(game->quizBtnB);
    if (game->quizBtnC) SDL_DestroyTexture(game->quizBtnC);
    if (game->quizFont) TTF_CloseFont(game->quizFont);
    if (game->quizMusic) Mix_FreeMusic(game->quizMusic);
    if (game->quizBeep) Mix_FreeChunk(game->quizBeep);
    if (game->quizBeep2) Mix_FreeChunk(game->quizBeep2);
    if (game->quizLaugh) Mix_FreeChunk(game->quizLaugh);

    StartPlay_Cleanup();

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}
