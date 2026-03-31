#include "header.h"
#include <string.h>

void Menu_LectureEntree(Game *game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { game->running = 0; return; }

        /* Hover detection */
        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x, my = event.motion.y;
            for (int i = 0; i < 5; i++) {
                int prev = game->buttons[i].selected;
                game->buttons[i].selected =
                    SDL_PointInRect(&(SDL_Point){mx, my}, &game->buttons[i].rect);
                /* Play hover sound once on enter */
                if (game->buttons[i].selected && !prev)
                    Mix_PlayChannel(-1, game->Sound, 0);
            }
        }

        /* Click detection */
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;
            /* Button 0 → Play */
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[0].rect))
                game->currentState = STATE_GAME;
            /* Button 2 → Scores / Leaderboard  ← KEY INTEGRATION POINT */
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[2].rect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_SCORES;
            }
            /* Button 4 → Quit */
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[4].rect))
                game->running = 0;
        }
    }
}

void Menu_Affichage(Game *game, SDL_Renderer *renderer)
{
    if (game->background)  SDL_RenderCopy(renderer, game->background,  NULL, NULL);
    SDL_RenderCopy(renderer, game->titleTexture, NULL, &game->titleRect);
    SDL_RenderCopy(renderer, game->logoTexture,  NULL, &game->logoRect);
    SDL_RenderCopy(renderer, game->trapTexture,  NULL, &game->trapRect);

    for (int i = 0; i < 5; i++) {
        SDL_Texture *t = game->buttons[i].selected
                         ? game->buttons[i].hoverTex
                         : game->buttons[i].normalTex;
        SDL_RenderCopy(renderer, t, NULL, &game->buttons[i].rect);
    }
}
