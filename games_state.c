#include "game.h"
#include "assets_catalog.h"
#include <stdio.h>

static SDL_Rect quizBtnARect = {120, 430, 150, 120};
static SDL_Rect quizBtnBRect = {325, 430, 150, 120};
static SDL_Rect quizBtnCRect = {530, 430, 150, 120};
static int quizHoverA = 0, quizHoverB = 0, quizHoverC = 0;
static int quizSelected = -1;

static int point_in_rect(SDL_Rect r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

static void draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                             SDL_Color color, SDL_Rect box) {
    if (!font || !text || !*text) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        SDL_Rect dst = {
            box.x + (box.w - surf->w) / 2,
            box.y + (box.h - surf->h) / 2,
            surf->w,
            surf->h
        };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

int Games_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->gamesLoaded) return 1;

    game->quizBg1 = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_1);
    game->quizBg2 = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_2);
    game->quizBtnA = IMG_LoadTexture(renderer, QUIZ_BUTTON_A);
    game->quizBtnB = IMG_LoadTexture(renderer, QUIZ_BUTTON_B);
    game->quizBtnC = IMG_LoadTexture(renderer, QUIZ_BUTTON_C);
    game->quizMusic = Mix_LoadMUS(GAME_ASSETS.songs.quiz_music);
    game->quizBeep = Mix_LoadWAV(SOUND_QUIZ_BEEP_1);
    game->quizBeep2 = Mix_LoadWAV(SOUND_QUIZ_BEEP_2);
    game->quizLaugh = Mix_LoadWAV(SOUND_QUIZ_LAUGH);
    game->quizFont = TTF_OpenFont(GAME_ASSETS.fonts.quiz_primary, 26);
    if (!game->quizFont) game->quizFont = TTF_OpenFont(GAME_ASSETS.fonts.quiz_fallback, 26);

    quizSelected = -1;
    quizHoverA = quizHoverB = quizHoverC = 0;

    if (game->quizMusic) {
        Mix_VolumeMusic(40);
        Mix_PlayMusic(game->quizMusic, -1);
    }

    game->gamesLoaded = 1;
    printf("Enigme/Quiz assets charges\n");
    return 1;
}

void Games_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            return;
        }

        if (e.type == SDL_MOUSEMOTION && game->currentState == STATE_ENIGME_QUIZ) {
            int mx = e.motion.x, my = e.motion.y;
            quizHoverA = point_in_rect(quizBtnARect, mx, my);
            quizHoverB = point_in_rect(quizBtnBRect, mx, my);
            quizHoverC = point_in_rect(quizBtnCRect, mx, my);
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            if (game->currentState == STATE_ENIGME_QUIZ) {
                if (point_in_rect(quizBtnARect, mx, my)) {
                    quizSelected = 0;
                    if (game->quizBeep) Mix_PlayChannel(-1, game->quizBeep, 0);
                    if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                }
                if (point_in_rect(quizBtnBRect, mx, my)) {
                    quizSelected = 1;
                    if (game->quizBeep2) Mix_PlayChannel(-1, game->quizBeep2, 0);
                }
                if (point_in_rect(quizBtnCRect, mx, my)) {
                    quizSelected = 2;
                    if (game->quizBeep) Mix_PlayChannel(-1, game->quizBeep, 0);
                    if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                }
            } else {
                game->currentState = STATE_ENIGME_QUIZ;
            }
        }
    }
}

void Games_Affichage(Game *game, SDL_Renderer *renderer) {
    if (game->currentState == STATE_ENIGME_QUIZ) {
        if (game->quizBg2) SDL_RenderCopy(renderer, game->quizBg2, NULL, NULL);
        else if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);

        if (game->quizFont) {
            SDL_Color black = {0, 0, 0, 255};
            SDL_Color white = {255, 255, 255, 255};
            draw_center_text(renderer, game->quizFont,
                             "QUEL OBJET KEVIN MET-IL SUR LA POIGNEE ?",
                             black, (SDL_Rect){100, 70, WIDTH - 200, 70});
            draw_center_text(renderer, game->quizFont,
                             "A: Bouilloire   B: Fer a repasser   C: Radiateur",
                             white, (SDL_Rect){50, 140, WIDTH - 100, 40});
        }

        SDL_Rect a = quizBtnARect, b = quizBtnBRect, c = quizBtnCRect;
        if (quizHoverA) a.y -= 4;
        if (quizHoverB) b.y -= 4;
        if (quizHoverC) c.y -= 4;

        if (game->quizBtnA) SDL_RenderCopy(renderer, game->quizBtnA, NULL, &a);
        if (game->quizBtnB) SDL_RenderCopy(renderer, game->quizBtnB, NULL, &b);
        if (game->quizBtnC) SDL_RenderCopy(renderer, game->quizBtnC, NULL, &c);

        if (quizSelected != -1 && game->quizFont) {
            SDL_Color col = (quizSelected == 1)
                ? (SDL_Color){30, 220, 30, 255}
                : (SDL_Color){240, 30, 30, 255};
            const char *msg = (quizSelected == 1) ? "BRAVO ! Bonne reponse." : "FAUX ! Reessayez.";
            draw_center_text(renderer, game->quizFont, msg, col, (SDL_Rect){120, 280, WIDTH - 240, 50});
        }
    } else {
        if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);
    }
}

void Games_MiseAJour(Game *game) {
    (void)game;
    SDL_Delay(16);
}
