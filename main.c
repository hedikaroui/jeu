#include "game.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    Game game;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!Initialisation(&game, &window, &renderer)) {
        fprintf(stderr, "Initialisation echouee\n");
        return 1;
    }

    while (game.running) {
        switch (game.currentState) {
            case STATE_MENU:
                Menu_LectureEntree(&game);
                SDL_RenderClear(renderer);
                Menu_Affichage(&game, renderer);
                SDL_RenderPresent(renderer);
                break;

            case STATE_OPTIONS:
                if (!game.optionsLoaded) Options_Charger(&game, renderer);
                Options_LectureEntree(&game);
                Options_Affichage(&game, renderer);
                Options_MiseAJour(&game);
                break;

            case STATE_SAVE:
            case STATE_SAVE_CHOICE:
                if (!game.saveBg) Save_Charger(&game, renderer);
                Save_LectureEntree(&game);
                Save_Affichage(&game, renderer);
                Save_MiseAJour(&game);
                break;

            case STATE_PLAYER:
            case STATE_PLAYER_CONFIG:
                if (!game.ps_loaded) PlayerSelect_Charger(&game, renderer);
                PlayerSelect_LectureEntree(&game);
                PlayerSelect_Affichage(&game, renderer);
                PlayerSelect_MiseAJour(&game);
                break;

            case STATE_SCORES_INPUT:
            case STATE_SCORES_LIST:
                Leaderboard_LectureEntree(&game);
                SDL_RenderClear(renderer);
                Leaderboard_Affichage(&game, renderer);
                SDL_RenderPresent(renderer);
                break;

            case STATE_START_PLAY:
                if (!game.startPlayLoaded) StartPlay_Charger(&game, renderer);
                StartPlay_LectureEntree(&game);
                StartPlay_Affichage(&game, renderer);
                StartPlay_MiseAJour(&game);
                break;

            case STATE_ENIGME:
            case STATE_ENIGME_QUIZ:
                if (!game.gamesLoaded) Games_Charger(&game, renderer);
                Games_LectureEntree(&game);
                Games_Affichage(&game, renderer);
                Games_MiseAJour(&game);
                break;

            case STATE_HISTOIRE:
            case STATE_GAME:
                game.currentState = STATE_SAVE;
                break;

            case STATE_QUIT:
            default:
                game.running = 0;
                break;
        }
    }

    Liberation(&game, window, renderer);
    return 0;
}
