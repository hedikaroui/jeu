#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

GameState Game_MainStateFromSubState(GameSubState subState) {
    switch (subState) {
        case STATE_GAME:
        case STATE_START_PLAY:
        case STATE_ENIGME:
        case STATE_ENIGME_QUIZ:
            return MAIN_STATE_GAME;

        case STATE_SCORES_INPUT:
        case STATE_SCORES_LIST:
            return MAIN_STATE_SCORE;

        case STATE_HISTOIRE:
            return MAIN_STATE_HISTORY;

        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
        case STATE_DISPLAY_CHOICE:
            return MAIN_STATE_PLAYER;

        case STATE_MENU:
        case STATE_SKIN_SELECT:
        case STATE_OPTIONS:
        case STATE_SAVE:
        case STATE_SAVE_CHOICE:
        case STATE_QUIT:
        default:
            return MAIN_STATE_MENU;
    }
}

void Game_SetSubState(Game *game, GameSubState subState) {
    if (!game) return;
    game->currentSubState = subState;
    game->currentState = Game_MainStateFromSubState(subState);
}

void Game_ResetRuntime(Game *game) {
    if (!game) return;

    game->gameLoaded = 0;
    game->gameLastTick = 0;
    game->gameCharacter.up = 0;
    game->gameCharacter.jumpPhase = 0;
    game->gameCharacter.posinit = 0;
    game->gameCharacter.posinitX = 0;
    game->gameCharacter.jumpRelX = -50.0;
    game->gameCharacter.jumpRelY = 0.0;
    game->gameCharacter.jumpProgress = 0.0;
    game->gameCharacter.jumpDir = 0;
    game->gameCharacter.groundY = 0;
    game->gameCharacter.facing = 1;
    game->gameCharacter.moving = 0;
    game->gameCharacter.movementState = 0;
    game->gameCharacter.pendingJump = 0;
    game->gameCharacter.frameIndex = 0;
    game->gameCharacter.lastFrameTick = 0;
    game->gameCharacter.damageActive = 0;
    game->gameCharacter.damageStartTick = 0;
    game->gameCharacter.damageInvulnUntil = 0;
    game->gameCharacter.energy = 0.0;
    game->gameCharacter.tiredUntil = 0;
    game->gameCharacter.pickupActive = 0;
    game->gameCharacter.pickupStartTick = 0;
    game->gameCharacter.pickupPendingSnowball = 0;
    game->gameCharacter.hasSnowball = 0;

    game->gameCharacter2.up = 0;
    game->gameCharacter2.jumpPhase = 0;
    game->gameCharacter2.posinit = 0;
    game->gameCharacter2.posinitX = 0;
    game->gameCharacter2.jumpRelX = -50.0;
    game->gameCharacter2.jumpRelY = 0.0;
    game->gameCharacter2.jumpProgress = 0.0;
    game->gameCharacter2.jumpDir = 0;
    game->gameCharacter2.groundY = 0;
    game->gameCharacter2.facing = -1;
    game->gameCharacter2.moving = 0;
    game->gameCharacter2.movementState = 0;
    game->gameCharacter2.pendingJump = 0;
    game->gameCharacter2.frameIndex = 0;
    game->gameCharacter2.lastFrameTick = 0;
    game->gameCharacter2.damageActive = 0;
    game->gameCharacter2.damageStartTick = 0;
    game->gameCharacter2.damageInvulnUntil = 0;
    game->gameCharacter2.energy = 0.0;
    game->gameCharacter2.tiredUntil = 0;
    game->gameCharacter2.pickupActive = 0;
    game->gameCharacter2.pickupStartTick = 0;
    game->gameCharacter2.pickupPendingSnowball = 0;
    game->gameCharacter2.hasSnowball = 0;

    game->gameEnemy.active = 0;
    for (int i = 0; i < GAME_OBSTACLE_COUNT; i++) {
        game->gameObstacles[i].active = 0;
        game->gameObstacles[i].collidingPlayer1 = 0;
        game->gameObstacles[i].collidingPlayer2 = 0;
        game->gameObstacles[i].state = 0;
        game->gameObstacles[i].stateTick = 0;
    }
}

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

    srand((unsigned int)(time(NULL) ^ SDL_GetPerformanceCounter()));

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
    game->miniMapFrameTex = NULL;
    game->miniMapLockClosedTex = NULL;
    game->miniMapLockOpenTex = NULL;
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
    game->solo_selected_player = 0;
    game->duo_display_mode = 0;
    game->duo_background_mode = 0;
    game->minimap_zoom = 1.0f;
    game->playerControls[0] = GAME_CONTROL_KEYBOARD;
    game->playerControls[1] = GAME_CONTROL_KEYBOARD;
    game->keyBindings[0][KEY_ACTION_WALK] = SDL_SCANCODE_D;
    game->keyBindings[0][KEY_ACTION_JUMP] = SDL_SCANCODE_W;
    game->keyBindings[0][KEY_ACTION_RUN] = SDL_SCANCODE_LSHIFT;
    game->keyBindings[0][KEY_ACTION_DOWN] = SDL_SCANCODE_A;
    game->keyBindings[0][KEY_ACTION_DANCE] = SDL_SCANCODE_SPACE;
    game->keyBindings[1][KEY_ACTION_WALK] = SDL_SCANCODE_RIGHT;
    game->keyBindings[1][KEY_ACTION_JUMP] = SDL_SCANCODE_UP;
    game->keyBindings[1][KEY_ACTION_RUN] = SDL_SCANCODE_RSHIFT;
    game->keyBindings[1][KEY_ACTION_DOWN] = SDL_SCANCODE_LEFT;
    game->keyBindings[1][KEY_ACTION_DANCE] = SDL_SCANCODE_RETURN;
    memset(&game->ps_bg, 0, sizeof(game->ps_bg));

    game->msGreTex = IMG_LoadTexture(*renderer, SCORE_BACK_HOVER);
    game->msRedTex = IMG_LoadTexture(*renderer, SCORE_BACK_NORMAL);
    game->leaderTex = IMG_LoadTexture(*renderer, GAME_ASSETS.backgrounds.leaderboard);
    game->click = Mix_LoadWAV(SOUND_CLICK);
    game->scoreJ1Tex = IMG_LoadTexture(*renderer, SCORE_BUTTON_NORMAL);
    game->scoreJ2Tex = IMG_LoadTexture(*renderer, SCORE_BUTTON_HOVER);
    game->startPlayer1LifeTex = NULL;
    game->startPlayer2LifeTex = NULL;
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
    game->skinSelectLoaded = 0;
    game->selectedEnemySkinIndex = -1;
    game->selectedEnemySkinPath[0] = '\0';
    game->skinSelectReturnState = STATE_START_PLAY;
    memset(&game->gameCharacter, 0, sizeof(game->gameCharacter));
    Game_ResetRuntime(game);

    game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 32);
    if (!game->font)
        game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 32);

    if (game->music) Mix_PlayMusic(game->music, -1);

    game->window = *window;
    game->volume = Mix_VolumeMusic(-1);
    game->fullscreen = 0;
    Game_SetSubState(game, STATE_MENU);
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
    if (game->miniMapFrameTex) SDL_DestroyTexture(game->miniMapFrameTex);
    if (game->miniMapLockClosedTex) SDL_DestroyTexture(game->miniMapLockClosedTex);
    if (game->miniMapLockOpenTex) SDL_DestroyTexture(game->miniMapLockOpenTex);
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
    if (game->startPlayer1LifeTex) SDL_DestroyTexture(game->startPlayer1LifeTex);
    if (game->startPlayer2LifeTex) SDL_DestroyTexture(game->startPlayer2LifeTex);
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
    for (int i = 0; i < 2; i++) {
        if (game->puzzlePictureTex[i]) SDL_DestroyTexture(game->puzzlePictureTex[i]);
        for (int j = 0; j < 3; j++) {
            if (game->puzzlePieceTex[i][j]) SDL_DestroyTexture(game->puzzlePieceTex[i][j]);
        }
    }
    if (game->quizFont) TTF_CloseFont(game->quizFont);
    if (game->quizMusic) Mix_FreeMusic(game->quizMusic);
    if (game->quizBeep) Mix_FreeChunk(game->quizBeep);
    if (game->quizBeep2) Mix_FreeChunk(game->quizBeep2);
    if (game->quizLaugh) Mix_FreeChunk(game->quizLaugh);

    if (game->gameCharacter.idleTexture) SDL_DestroyTexture(game->gameCharacter.idleTexture);
    if (game->gameCharacter.idleBackTexture) SDL_DestroyTexture(game->gameCharacter.idleBackTexture);
    if (game->gameCharacter.walkTexture) SDL_DestroyTexture(game->gameCharacter.walkTexture);
    if (game->gameCharacter.walkBackTexture) SDL_DestroyTexture(game->gameCharacter.walkBackTexture);
    if (game->gameCharacter.runTexture) SDL_DestroyTexture(game->gameCharacter.runTexture);
    if (game->gameCharacter.runBackTexture) SDL_DestroyTexture(game->gameCharacter.runBackTexture);
    if (game->gameCharacter.jumpTexture) SDL_DestroyTexture(game->gameCharacter.jumpTexture);
    if (game->gameCharacter.jumpBackTexture) SDL_DestroyTexture(game->gameCharacter.jumpBackTexture);
    if (game->gameCharacter.damageTexture) SDL_DestroyTexture(game->gameCharacter.damageTexture);
    if (game->gameCharacter.layDownTexture) SDL_DestroyTexture(game->gameCharacter.layDownTexture);
    if (game->gameCharacter.tiredTexture) SDL_DestroyTexture(game->gameCharacter.tiredTexture);
    if (game->gameCharacter.pickupTexture) SDL_DestroyTexture(game->gameCharacter.pickupTexture);
    if (game->gameCharacter2.idleTexture) SDL_DestroyTexture(game->gameCharacter2.idleTexture);
    if (game->gameCharacter2.idleBackTexture) SDL_DestroyTexture(game->gameCharacter2.idleBackTexture);
    if (game->gameCharacter2.walkTexture) SDL_DestroyTexture(game->gameCharacter2.walkTexture);
    if (game->gameCharacter2.walkBackTexture) SDL_DestroyTexture(game->gameCharacter2.walkBackTexture);
    if (game->gameCharacter2.runTexture) SDL_DestroyTexture(game->gameCharacter2.runTexture);
    if (game->gameCharacter2.runBackTexture) SDL_DestroyTexture(game->gameCharacter2.runBackTexture);
    if (game->gameCharacter2.jumpTexture) SDL_DestroyTexture(game->gameCharacter2.jumpTexture);
    if (game->gameCharacter2.jumpBackTexture) SDL_DestroyTexture(game->gameCharacter2.jumpBackTexture);
    if (game->gameCharacter2.damageTexture) SDL_DestroyTexture(game->gameCharacter2.damageTexture);
    if (game->gameCharacter2.layDownTexture) SDL_DestroyTexture(game->gameCharacter2.layDownTexture);
    if (game->gameCharacter2.tiredTexture) SDL_DestroyTexture(game->gameCharacter2.tiredTexture);
    if (game->gameCharacter2.pickupTexture) SDL_DestroyTexture(game->gameCharacter2.pickupTexture);
    if (game->gameEnemyTex) SDL_DestroyTexture(game->gameEnemyTex);
    if (game->gameEnemyStandTex) SDL_DestroyTexture(game->gameEnemyStandTex);
    if (game->gameEnemyWalkTex) SDL_DestroyTexture(game->gameEnemyWalkTex);
    if (game->gameEnemyRunTex) SDL_DestroyTexture(game->gameEnemyRunTex);
    if (game->gameSpiderTex) SDL_DestroyTexture(game->gameSpiderTex);
    if (game->gameFallingTex) SDL_DestroyTexture(game->gameFallingTex);
    if (game->gameObstacleHitSound) Mix_FreeChunk(game->gameObstacleHitSound);

    StartPlay_Cleanup();
    SkinSelect_Cleanup();

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

int joueurs_non_configures(const Game *game) {
    if (!game || !game->ps_loaded) return 1;
    if (strlen(game->player1_name) == 0) return 1;
    return game->player_mode != 1 && strlen(game->player2_name) == 0;
}

void GameLoop_ModuleInitialisationEtat(Game *game, SDL_Renderer *renderer) {
    switch (game->currentSubState) {
        case STATE_SKIN_SELECT:
            if (!game->skinSelectLoaded) SkinSelect_Charger(game, renderer);
            break;

        case STATE_OPTIONS:
            if (!game->optionsLoaded) Options_Charger(game, renderer);
            break;

        case STATE_SAVE:
        case STATE_SAVE_CHOICE:
            if (!game->saveBg) Save_Charger(game, renderer);
            break;

        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            if (!game->ps_loaded) PlayerSelect_Charger(game, renderer);
            break;

        case STATE_DISPLAY_CHOICE:
            if (!game->ps_loaded) PlayerSelect_Charger(game, renderer);
            break;

        case STATE_START_PLAY:
            if (game->selectedEnemySkinPath[0] == '\0') {
                game->skinSelectReturnState = STATE_START_PLAY;
                Game_SetSubState(game, STATE_SKIN_SELECT);
                break;
            }
            if (!game->startPlayLoaded) StartPlay_Charger(game, renderer);
            break;

        case STATE_ENIGME:
        case STATE_ENIGME_QUIZ:
            if (!game->gamesLoaded) Games_Charger(game, renderer);
            break;

        case STATE_GAME:
            if (game->selectedEnemySkinPath[0] == '\0') {
                game->skinSelectReturnState = STATE_GAME;
                Game_SetSubState(game, STATE_SKIN_SELECT);
                break;
            }
            if (joueurs_non_configures(game)) PlayerSelect_Charger(game, renderer);
            else if (!game->gameLoaded) Game_Charger(game, renderer);
            break;

        case STATE_MENU:
        case STATE_SCORES_INPUT:
        case STATE_SCORES_LIST:
        case STATE_HISTOIRE:
        case STATE_QUIT:
        default:
            break;
    }
}

void GameLoop_ModuleInput(Game *game, SDL_Renderer *renderer) {
    GameLoop_ModuleInitialisationEtat(game, renderer);

    switch (game->currentSubState) {
        case STATE_SKIN_SELECT:
            SkinSelect_LectureEntree(game);
            break;

        case STATE_MENU:
            Menu_LectureEntree(game);
            break;

        case STATE_OPTIONS:
            Options_LectureEntree(game);
            break;

        case STATE_SAVE:
        case STATE_SAVE_CHOICE:
            Save_LectureEntree(game);
            break;

        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            PlayerSelect_LectureEntree(game);
            break;

        case STATE_DISPLAY_CHOICE:
            DisplayChoice_LectureEntree(game);
            break;

        case STATE_SCORES_INPUT:
        case STATE_SCORES_LIST:
            Leaderboard_LectureEntree(game);
            break;

        case STATE_START_PLAY:
            StartPlay_LectureEntree(game);
            break;

        case STATE_ENIGME:
        case STATE_ENIGME_QUIZ:
            Games_LectureEntree(game);
            break;

        case STATE_HISTOIRE:
            Game_SetSubState(game, STATE_MENU);
            break;

        case STATE_GAME:
            if (joueurs_non_configures(game)) PlayerSelect_LectureEntree(game);
            else Game_LectureEntree(game);
            break;

        case STATE_QUIT:
        default:
            break;
    }
}

void GameLoop_ModuleUpdate(Game *game) {
    switch (game->currentSubState) {
        case STATE_SKIN_SELECT:
            SkinSelect_MiseAJour(game);
            break;

        case STATE_MENU:
            Menu_MiseAJour(game);
            break;

        case STATE_OPTIONS:
            Options_MiseAJour(game);
            break;

        case STATE_SAVE:
        case STATE_SAVE_CHOICE:
            Save_MiseAJour(game);
            break;

        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            PlayerSelect_MiseAJour(game);
            break;

        case STATE_DISPLAY_CHOICE:
            DisplayChoice_MiseAJour(game);
            break;

        case STATE_START_PLAY:
            StartPlay_MiseAJour(game);
            break;

        case STATE_ENIGME:
        case STATE_ENIGME_QUIZ:
            Games_MiseAJour(game);
            break;

        case STATE_GAME:
            if (joueurs_non_configures(game)) PlayerSelect_MiseAJour(game);
            else Game_MiseAJour(game);
            break;

        case STATE_HISTOIRE:
        case STATE_SCORES_INPUT:
        case STATE_SCORES_LIST:
            break;
        case STATE_QUIT:
            game->running = 0;
            break;
        default:
            break;
    }
}

void GameLoop_ModuleAffichage(Game *game, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    switch (game->currentSubState) {
        case STATE_SKIN_SELECT:
            SkinSelect_Affichage(game, renderer);
            break;

        case STATE_MENU:
            Menu_Affichage(game, renderer);
            break;

        case STATE_OPTIONS:
            Options_Affichage(game, renderer);
            break;

        case STATE_SAVE:
        case STATE_SAVE_CHOICE:
            Save_Affichage(game, renderer);
            break;

        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            PlayerSelect_Affichage(game, renderer);
            break;

        case STATE_DISPLAY_CHOICE:
            DisplayChoice_Affichage(game, renderer);
            break;

        case STATE_SCORES_INPUT:
        case STATE_SCORES_LIST:
            Leaderboard_Affichage(game, renderer);
            break;

        case STATE_START_PLAY:
            StartPlay_Affichage(game, renderer);
            break;

        case STATE_ENIGME:
        case STATE_ENIGME_QUIZ:
            Games_Affichage(game, renderer);
            break;

        case STATE_GAME:
            if (joueurs_non_configures(game)) PlayerSelect_Affichage(game, renderer);
            else Game_Affichage(game, renderer);
            break;

        case STATE_HISTOIRE:
        case STATE_QUIT:
        default:
            break;
    }

    if (game->currentSubState != STATE_QUIT) {
        SDL_RenderPresent(renderer);
    }
}
