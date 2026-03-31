/* ═══════════════════════════════════════════════════════════════════
   OPTIONS – Integrated into main project (merged from menu_option.c)
═══════════════════════════════════════════════════════════════════ */

#include "game.h"
#include <stdio.h>

/* Helper: check if point is in rect */
static int mouse_over(SDL_Rect r, int x, int y) {
    return (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h);
}

/* ═══════════════════════════════════════════════
   OPTIONS – CHARGEMENT DES ASSETS
═══════════════════════════════════════════════ */
int Options_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->optionsLoaded) return 1;

    game->optionsBg          = IMG_LoadTexture(renderer, "options.png");
    game->volumePlusBtn      = IMG_LoadTexture(renderer, "volumeplus.png");
    game->volumeMinusBtn     = IMG_LoadTexture(renderer, "volumeminus.png");
    game->volumeMuteBtn      = IMG_LoadTexture(renderer, "volumemute.png");
    game->fullscreenBtn      = IMG_LoadTexture(renderer, "fullscreen.png");
    game->normalscreenBtn    = IMG_LoadTexture(renderer, "normalscreen.png");
    game->optionsClick       = Mix_LoadWAV("sonbref.wav");

    /* Create OPTIONS title texture */
    TTF_Font *f = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 60);
    if (!f) f = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 60);
    if (f) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface *surf = TTF_RenderUTF8_Blended(f, "OPTIONS", white);
        if (surf) {
            game->optionsTitle = SDL_CreateTextureFromSurface(renderer, surf);
            game->optionsTitleRect = (SDL_Rect){(WIDTH - surf->w) / 2, 40, surf->w, 80};
            SDL_FreeSurface(surf);
        }
        TTF_CloseFont(f);
    }

    game->optionsFullscreen = 0;
    game->optionsLoaded = 1;
    printf("Options: assets charges\n");
    return 1;
}

/* ═══════════════════════════════════════════════
   OPTIONS – INPUT
═══════════════════════════════════════════════ */
void Options_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            return;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;

            /* Button rects (centered) */
            int centerX = WIDTH / 2;
            int bw = 80, bh = 80;
            SDL_Rect btnMinus = {centerX - 180, HEIGHT / 2, bw, bh};
            SDL_Rect btnMute  = {centerX - 40,  HEIGHT / 2, bw, bh};
            SDL_Rect btnPlus  = {centerX + 100, HEIGHT / 2, bw, bh};
            SDL_Rect btnFull  = {centerX - 40,  HEIGHT / 2 + 120, bw, bh};

            if (game->optionsClick)
                Mix_PlayChannel(-1, game->optionsClick, 0);

            /* Volume minus */
            if (mouse_over(btnMinus, mx, my)) {
                int vol = Mix_VolumeMusic(-1) - 10;
                if (vol < 0) vol = 0;
                Mix_VolumeMusic(vol);
            }
            /* Volume mute */
            if (mouse_over(btnMute, mx, my)) {
                Mix_VolumeMusic(0);
            }
            /* Volume plus */
            if (mouse_over(btnPlus, mx, my)) {
                int vol = Mix_VolumeMusic(-1) + 10;
                if (vol > 128) vol = 128;
                Mix_VolumeMusic(vol);
            }
            /* Fullscreen toggle */
            if (mouse_over(btnFull, mx, my)) {
                game->optionsFullscreen = !game->optionsFullscreen;
                /* Note: Fullscreen toggle would require window reference, 
                   skipped for now but stored in optionsFullscreen flag */
            }
        }
    }
}

/* ═══════════════════════════════════════════════
   OPTIONS – RENDU
═══════════════════════════════════════════════ */
void Options_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderClear(renderer);

    /* Draw background */
    if (game->optionsBg)
        SDL_RenderCopy(renderer, game->optionsBg, NULL, NULL);

    /* Draw title */
    if (game->optionsTitle)
        SDL_RenderCopy(renderer, game->optionsTitle, NULL, &game->optionsTitleRect);

    /* Button positions (centered) */
    int centerX = WIDTH / 2;
    int bw = 80, bh = 80;
    SDL_Rect btnMinus = {centerX - 180, HEIGHT / 2, bw, bh};
    SDL_Rect btnMute  = {centerX - 40,  HEIGHT / 2, bw, bh};
    SDL_Rect btnPlus  = {centerX + 100, HEIGHT / 2, bw, bh};
    SDL_Rect btnFull  = {centerX - 40,  HEIGHT / 2 + 120, bw, bh};

    /* Draw volume buttons */
    if (game->volumeMinusBtn)
        SDL_RenderCopy(renderer, game->volumeMinusBtn, NULL, &btnMinus);
    if (game->volumeMuteBtn)
        SDL_RenderCopy(renderer, game->volumeMuteBtn, NULL, &btnMute);
    if (game->volumePlusBtn)
        SDL_RenderCopy(renderer, game->volumePlusBtn, NULL, &btnPlus);

    /* Draw fullscreen toggle button */
    if (game->optionsFullscreen && game->normalscreenBtn)
        SDL_RenderCopy(renderer, game->normalscreenBtn, NULL, &btnFull);
    else if (!game->optionsFullscreen && game->fullscreenBtn)
        SDL_RenderCopy(renderer, game->fullscreenBtn, NULL, &btnFull);

    SDL_RenderPresent(renderer);
}

/* ═══════════════════════════════════════════════
   OPTIONS – MISE A JOUR
═══════════════════════════════════════════════ */
void Options_MiseAJour(Game *game) {
    SDL_Delay(16);
}
