#include "header.h"

void Jeu_LectureEntree(Game *game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { game->running = 0; return; }

        /* ESC returns to menu */
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            return;
            
        }

        /* Hover detection */
        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x, my = event.motion.y;
            for (int i = 0; i < 5; i++) {
                int prev = game->gameButtons[i].selected;
                game->gameButtons[i].selected = SDL_PointInRect(&(SDL_Point){mx, my}, &game->gameButtons[i].rect);
                /* Play hover sound once on enter */
                if (game->gameButtons[i].selected && !prev)
                    Mix_PlayChannel(-1, game->Sound, 0);
            }
        }

        /* Click detection: any game button → go to scores */
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;
            for (int i = 0; i < 5; i++) {
                if (SDL_PointInRect(&(SDL_Point){mx, my}, &game->gameButtons[i].rect)) {
                    HandleGameButtonClick(game);
                    return;
                }
            }
        }
    }
}

void Jeu_Affichage(Game *game, SDL_Renderer *renderer)
{
    /* Game background */
    if (game->background) SDL_RenderCopy(renderer, game->background, NULL, NULL);

    /* Draw all game buttons with hover effects */
    for (int i = 0; i < 5; i++) {
        SDL_Texture *t = game->gameButtons[i].selected
                         ? game->gameButtons[i].hoverTex
                         : game->gameButtons[i].normalTex;
        SDL_RenderCopy(renderer, t, NULL, &game->gameButtons[i].rect);
    }
}

void HandleGameButtonClick(Game *game)
{
    /* Play click sound effect */
    if (game->click)
        Mix_PlayChannel(-1, game->click, 0);
    
    /* Transition from game state to score state */
    game->currentState = STATE_SCORES;
}
