#include "game.h"
#include <string.h>

static int joueurs_non_configures(const Game *game) {
    return !game->ps_loaded ||
           strlen(game->player1_name) == 0 ||
           strlen(game->player2_name) == 0;
}

void GameLoop_ModuleInitialisationEtat(Game *game, SDL_Renderer *renderer) {
    switch (game->currentSubState) {
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

        case STATE_START_PLAY:
            if (!game->startPlayLoaded) StartPlay_Charger(game, renderer);
            break;

        case STATE_ENIGME:
        case STATE_ENIGME_QUIZ:
            if (!game->gamesLoaded) Games_Charger(game, renderer);
            break;

        case STATE_GAME:
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

void GameLoop_ModuleInput(Game *game) {
    switch (game->currentSubState) {
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
        case STATE_QUIT:
        default:
            break;
    }
}

void GameLoop_ModuleAffichage(Game *game, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    switch (game->currentSubState) {
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
