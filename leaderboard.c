#include "game.h"
#include <stdio.h>
#include <string.h>

static SDL_Rect scoreStateJBtnRect = {0, 0, 0, 0};
static int scoreStateJBtnHover = 0;

static void leaderboard_sync_layout(Game *game) {
    int w = WIDTH;
    int h = HEIGHT;
    if (game->window) SDL_GetWindowSize(game->window, &w, &h);
    game->searchBox = (SDL_Rect){(w - 400) / 2, 140, 400, 50};
    scoreStateJBtnRect = (SDL_Rect){(w - 320) / 2, h - 130, 320, 90};
}

void Leaderboard_LectureEntree(Game *game) {
    SDL_Event e;
    leaderboard_sync_layout(game);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                game->inputActive = 0;
                SDL_StopTextInput();
                if (game->currentState == STATE_SCORES_LIST) {
                    game->currentState = STATE_QUIT;
                } else {
                    game->currentState = STATE_MENU;
                    if (game->music) Mix_PlayMusic(game->music, -1);
                }
                return;
            }
            if (e.key.keysym.sym == SDLK_e) {
                game->currentState = STATE_ENIGME_QUIZ;
                return;
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->backBtn.rect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_MENU;
                game->inputActive = 0;
                SDL_StopTextInput();
                if (game->music) Mix_PlayMusic(game->music, -1);
                return;
            }
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &scoreStateJBtnRect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_START_PLAY;
                return;
            }
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->searchBox)) {
                game->inputActive = 1;
                SDL_StartTextInput();
            } else {
                game->inputActive = 0;
                SDL_StopTextInput();
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            scoreStateJBtnHover = SDL_PointInRect(&(SDL_Point){e.motion.x, e.motion.y}, &scoreStateJBtnRect);
        }

        if (game->inputActive && e.type == SDL_TEXTINPUT) {
            if (strlen(game->inputText) + strlen(e.text.text) < sizeof(game->inputText)-1)
                strcat(game->inputText, e.text.text);
        }
        if (game->inputActive && e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(game->inputText) > 0)
                game->inputText[strlen(game->inputText)-1] = '\0';
            if (e.key.keysym.sym == SDLK_RETURN) {
                printf("Recherche joueur: %s\n", game->inputText);
                game->currentState = STATE_SCORES_LIST;
                return;
            }
        }
    }
}

void Leaderboard_Affichage(Game *game, SDL_Renderer *renderer) {
    leaderboard_sync_layout(game);

    if (game->leaderTex)
        SDL_RenderCopy(renderer, game->leaderTex, NULL, NULL);

    SDL_Texture *bt = game->backBtn.selected ? game->msGreTex : game->msRedTex;
    if (bt) SDL_RenderCopy(renderer, bt, NULL, &game->backBtn.rect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_RenderFillRect(renderer, &game->searchBox);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &game->searchBox);

    const char *display = strlen(game->inputText) > 0
                          ? game->inputText
                          : (game->inputActive ? "" : "Rechercher un joueur...");
    if (game->font && strlen(display) > 0) {
        SDL_Color col = strlen(game->inputText) > 0
                        ? (SDL_Color){255,255,255,255}
                        : (SDL_Color){180,180,180,255};
        SDL_Surface *surf = TTF_RenderUTF8_Blended(game->font, display, col);
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_Rect tr = {
                    game->searchBox.x + 10,
                    game->searchBox.y + (game->searchBox.h - surf->h) / 2,
                    surf->w < game->searchBox.w-20 ? surf->w : game->searchBox.w-20,
                    surf->h
                };
                SDL_RenderCopy(renderer, tex, NULL, &tr);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    {
        static Uint32 last_blink = 0;
        static int cursor_on = 1;
        Uint32 now = SDL_GetTicks();
        if (now - last_blink > 500) { cursor_on = !cursor_on; last_blink = now; }
        if (game->inputActive && cursor_on) {
            int cx = game->searchBox.x + 10;
            if (strlen(game->inputText) > 0 && game->font) {
                int tw; TTF_SizeUTF8(game->font, game->inputText, &tw, NULL);
                cx += tw;
            }
            SDL_SetRenderDrawColor(renderer, 255,255,255,255);
            SDL_RenderDrawLine(renderer, cx, game->searchBox.y+6, cx, game->searchBox.y+game->searchBox.h-6);
        }
    }

    {
        SDL_Rect drawRect = scoreStateJBtnRect;
        SDL_Texture *t = scoreStateJBtnHover ? game->scoreJ2Tex : game->scoreJ1Tex;
        if (t) {
            if (scoreStateJBtnHover) drawRect.y -= 4;
            SDL_RenderCopy(renderer, t, NULL, &drawRect);
        }
    }
}
