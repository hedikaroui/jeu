#include "game.h"
#include <stdio.h>

int main(void) {
    Game          game;
    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;

    if (!Initialisation(&game, &window, &renderer)) {
        printf("Erreur Initialisation\n");
        return 1;
    }

    while (game.running) {

        switch (game.currentState) {

            /* ── Menu principal ── */
            case STATE_MENU:
                Menu_LectureEntree(&game);
                SDL_RenderClear(renderer);
                Menu_Affichage(&game, renderer);
                SDL_RenderPresent(renderer);
                break;

            /* ── Selection des joueurs (Play) ── */
            case STATE_PLAYER_SELECT:
                if (!game.ps_loaded) PlayerSelect_Charger(&game, renderer);
                PlayerSelect_LectureEntree(&game);
                PlayerSelect_Affichage(&game, renderer);   /* contient SDL_RenderPresent */
                PlayerSelect_MiseAJour(&game);
                break;

            /* ── Jeu (intègre sélection joueurs si nécessaire) ── */
            case STATE_GAME:
                /* If player-select assets not loaded or names not set, run player-select UI here */
                if (!game.ps_loaded || strlen(game.player1_name) == 0 || strlen(game.player2_name) == 0) {
                    if (!game.ps_loaded) PlayerSelect_Charger(&game, renderer);
                    PlayerSelect_LectureEntree(&game);
                    PlayerSelect_Affichage(&game, renderer);   /* contient SDL_RenderPresent */
                    PlayerSelect_MiseAJour(&game);
                } else {
                    /* TODO: integrate actual game logic here */
                    SDL_Delay(16);
                }
                break;

            /* ── Sauvegarde ── */
            case STATE_SAVE:
                if (!game.saveBg) Save_Charger(&game, renderer);
                Save_LectureEntree(&game);
                Save_Affichage(&game, renderer);   /* contient SDL_RenderPresent */
                Save_MiseAJour(&game);
                break;

            /* ── Leaderboard / Scores ── */
            case STATE_SCORES:
                Leaderboard_LectureEntree(&game);
                SDL_RenderClear(renderer);
                Leaderboard_Affichage(&game, renderer);
                SDL_RenderPresent(renderer);
                break;

            /* ── Options ── */
            case STATE_OPTIONS:
                if (!game.optionsLoaded) Options_Charger(&game, renderer);
                Options_LectureEntree(&game);
                Options_Affichage(&game, renderer);   /* contient SDL_RenderPresent */
                Options_MiseAJour(&game);
                break;

            /* ── Games/Gifts ── */
            case STATE_GAMES:
                if (!game.gamesLoaded) Games_Charger(&game, renderer);
                Games_LectureEntree(&game);
                Games_Affichage(&game, renderer);       /* contient SDL_RenderPresent */
                Games_MiseAJour(&game);
                break;
        }
    }

    Liberation(&game, window, renderer);
    return 0;
}
