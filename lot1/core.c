#include "game.h"

#include <stdio.h>
#include <string.h>

GameState Game_MainStateFromSubState(GameSubState subState) {
    switch (subState) {
        case STATE_GAME:
        case STATE_START_PLAY:
        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
        case STATE_SCORES_INPUT:
        case STATE_SCORES_LIST:
        case STATE_MENU:
        default:
            return MAIN_STATE_PLAYER;
    }
}

void Game_SetSubState(Game *game, GameSubState subState) {
    if (!game) return;

    if (subState == STATE_MENU) subState = STATE_PLAYER;
    if (subState == STATE_SCORES_INPUT || subState == STATE_SCORES_LIST) {
        subState = STATE_START_PLAY;
    }

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

}

void lot1_init_controls(Game *game) {
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

    *window = SDL_CreateWindow("lot1 - Player Select + Play",
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

    game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 32);
    if (!game->font) {
        game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 32);
    }
    if (!game->font) game->font = TTF_OpenFont("fonts/arial.ttf", 32);

    game->player_mode = 2;
    game->solo_selected_player = 0;
    game->window = *window;
    game->volume = Mix_VolumeMusic(-1);
    game->fullscreen = 0;
    game->running = 1;

    lot1_init_controls(game);
    Game_ResetRuntime(game);
    Game_SetSubState(game, STATE_PLAYER);
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
    if (game->click) Mix_FreeChunk(game->click);

    if (game->startPlayer1LifeTex) SDL_DestroyTexture(game->startPlayer1LifeTex);
    if (game->startPlayer2LifeTex) SDL_DestroyTexture(game->startPlayer2LifeTex);
    if (game->startPlayer1SideViewTex) SDL_DestroyTexture(game->startPlayer1SideViewTex);
    if (game->startPlayer2SideViewTex) SDL_DestroyTexture(game->startPlayer2SideViewTex);
    if (game->startTextTex) SDL_DestroyTexture(game->startTextTex);

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
    if (game->gameCharacter.danceTexture) SDL_DestroyTexture(game->gameCharacter.danceTexture);
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
    if (game->gameCharacter2.danceTexture) SDL_DestroyTexture(game->gameCharacter2.danceTexture);
    StartPlay_Cleanup();

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

int lot1_players_missing(const Game *game) {
    if (!game || !game->ps_loaded) return 1;
    if (strlen(game->player1_name) == 0) return 1;
    return game->player_mode != 1 && strlen(game->player2_name) == 0;
}

void GameLoop_ModuleInitialisationEtat(Game *game, SDL_Renderer *renderer) {
    switch (game->currentSubState) {
        case STATE_MENU:
            Game_SetSubState(game, STATE_PLAYER);
            break;

        case STATE_SCORES_INPUT:
        case STATE_SCORES_LIST:
            Game_SetSubState(game, STATE_START_PLAY);
            if (!game->startPlayLoaded) StartPlay_Charger(game, renderer);
            break;

        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            if (!game->ps_loaded) PlayerSelect_Charger(game, renderer);
            break;

        case STATE_START_PLAY:
            if (!game->startPlayLoaded) StartPlay_Charger(game, renderer);
            break;

        case STATE_GAME:
            if (lot1_players_missing(game)) {
                Game_SetSubState(game, STATE_PLAYER);
                if (!game->ps_loaded) PlayerSelect_Charger(game, renderer);
            } else if (!game->gameLoaded) {
                Game_Charger(game, renderer);
            }
            break;

        case STATE_QUIT:
        default:
            break;
    }
}

void GameLoop_ModuleInput(Game *game, SDL_Renderer *renderer) {
    GameLoop_ModuleInitialisationEtat(game, renderer);

    switch (game->currentSubState) {
        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            PlayerSelect_LectureEntree(game);
            break;

        case STATE_START_PLAY:
            StartPlay_LectureEntree(game);
            break;

        case STATE_GAME:
            if (lot1_players_missing(game)) PlayerSelect_LectureEntree(game);
            else Game_LectureEntree(game);
            break;

        case STATE_QUIT:
            game->running = 0;
            break;

        default:
            Game_SetSubState(game, STATE_PLAYER);
            break;
    }
}

void GameLoop_ModuleUpdate(Game *game) {
    switch (game->currentSubState) {
        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            PlayerSelect_MiseAJour(game);
            break;

        case STATE_START_PLAY:
            StartPlay_MiseAJour(game);
            break;

        case STATE_GAME:
            if (lot1_players_missing(game)) PlayerSelect_MiseAJour(game);
            else Game_MiseAJour(game);
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
        case STATE_PLAYER:
        case STATE_PLAYER_CONFIG:
            PlayerSelect_Affichage(game, renderer);
            break;

        case STATE_START_PLAY:
            StartPlay_Affichage(game, renderer);
            break;

        case STATE_GAME:
            if (lot1_players_missing(game)) PlayerSelect_Affichage(game, renderer);
            else Game_Affichage(game, renderer);
            break;

        default:
            break;
    }

    if (game->currentSubState != STATE_QUIT) {
        SDL_RenderPresent(renderer);
    }
}
