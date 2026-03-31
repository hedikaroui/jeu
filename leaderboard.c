#include "header.h"
#include <string.h>

void Leaderboard_LectureEntree(Game *game)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        /* ESC always goes back to menu */
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            game->inputActive  = 0;
            SDL_StopTextInput();
            return;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;

            /* Back button → return to menu */
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->backBtn.rect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_MENU;
                game->inputActive  = 0;
                SDL_StopTextInput();
                return;
            }

            /* Search box focus */
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->searchBox)) {
                game->inputActive = 1;
                SDL_StartTextInput();
            } else {
                game->inputActive = 0;
                SDL_StopTextInput();
            }
        }

        /* Text typing into search box */
        if (game->inputActive && e.type == SDL_TEXTINPUT) {
            if (strlen(game->inputText) + strlen(e.text.text) < sizeof(game->inputText)-1)
                strcat(game->inputText, e.text.text);
        }
        if (game->inputActive && e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(game->inputText) > 0)
                game->inputText[strlen(game->inputText)-1] = '\0';
            if (e.key.keysym.sym == SDLK_RETURN)
                printf("Recherche joueur: %s\n", game->inputText);
        }
    }
}

void Leaderboard_Affichage(Game *game, SDL_Renderer *renderer)
{
    /* Full-screen leaderboard background */
    SDL_RenderCopy(renderer, game->leaderTex, NULL, NULL);

    /* Search box text */
    if (game->font && strlen(game->inputText) > 0) {
        SDL_Color noir  = {0, 0, 0, 255};
        SDL_Surface *surf = TTF_RenderText_Blended(game->font, game->inputText, noir);
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_RenderCopy(renderer, tex, NULL, &game->searchBox);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    /* Draw search box border so the user can see it even when empty */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &game->searchBox);

    /* Back button (use msRedTex as placeholder; swap for a dedicated texture if you have one) */
    if (game->msRedTex)
        SDL_RenderCopy(renderer, game->msRedTex, NULL, &game->backBtn.rect);
}
