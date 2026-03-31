/*
 * source.c retained for backward compatibility only.
 * Implementation has been refactored into smaller modules:
 *   assets.c, init.c, menu.c, leaderboard.c, game.c
 * The original contents were moved; this stub defines no symbols.
 */

#include "header.h"

/* empty legacy stub */

    /* --- SDL subsystems --- */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    /* --- Window & Renderer --- */
    *window = SDL_CreateWindow("Jeu Principal",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!*window) { fprintf(stderr, "Fenetre: %s\n", SDL_GetError()); return 0; }

    *renderer = SDL_CreateRenderer(*window, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) { fprintf(stderr, "Renderer: %s\n", SDL_GetError()); return 0; }

    /* ── Project-1 menu assets ── */
    game->background    = ChargerTexture(*renderer, "background.png");
    game->logoTexture   = ChargerTexture(*renderer, "logo.png");
    game->logoRect      = (SDL_Rect){250, 50, 300, 150};
    game->titleTexture  = ChargerTexture(*renderer, "logo.png");
    game->titleRect     = (SDL_Rect){150, 10, 500, 100};
    game->trapTexture   = ChargerTexture(*renderer, "background.png");
    game->trapRect      = (SDL_Rect){0, 0, WIDTH, HEIGHT};

    game->music = ChargerMusique("jingle.mp3");
    if (game->music) Mix_PlayMusic(game->music, -1);
    game->Sound = ChargerSon("magic.wav");

    /* Menu buttons: Play / Options / Scores / Credits / Quit */
    const char *normalFiles[] = {"m1.png","o1.png","s1.png","j1.png","h1.png"};
    const char *hoverFiles[]  = {"m2.png","o2.png","s2.png","j2.png","h2.png"};
    for (int i = 0; i < 5; i++) {
        game->buttons[i].normalTex = ChargerTexture(*renderer, normalFiles[i]);
        game->buttons[i].hoverTex  = ChargerTexture(*renderer, hoverFiles[i]);
        game->buttons[i].rect      = (SDL_Rect){150 + i*120, 250 + i*50, 100, 40};
        game->buttons[i].selected  = 0;
    }

    /* ── Project-2 leaderboard assets ── */
    game->msGreTex  = ChargerTexture(*renderer, "MSgre.png");
    game->msRedTex  = ChargerTexture(*renderer, "MSred.png");
    game->leaderTex = ChargerTexture(*renderer, "LB.jpg");
    game->click     = ChargerSon("click.mp3");

    /* Back button (top-right of leaderboard screen) */
    game->backBtn.rect     = (SDL_Rect){900, 10, 300, 120};
    game->backBtn.selected = 0;

    /* Game buttons - all click sequences lead to scores */
    const char *gameFiles[] = {"j1.png", "o1.png", "s1.png", "c1.png", "h1.png"};
    const char *gameHover[] = {"j2.png", "o2.png", "s2.png", "c2.png", "h2.png"};
    for (int i = 0; i < 5; i++) {
        game->gameButtons[i].normalTex = ChargerTexture(*renderer, gameFiles[i]);
        game->gameButtons[i].hoverTex  = ChargerTexture(*renderer, gameHover[i]);
        game->gameButtons[i].rect      = (SDL_Rect){150 + i*200, 300, 100, 40};
        game->gameButtons[i].selected  = 0;
    }

    /* Search box */
    game->searchBox    = (SDL_Rect){560, 180, 240, 40};
    game->inputActive  = 0;
    game->inputText[0] = '\0';

    /* Font (used for search box text) */
    game->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!game->font)
        fprintf(stderr, "Erreur police: %s\n", TTF_GetError());

    /* ── Initial state ── */
    game->currentState = STATE_MENU;
    game->running      = 1;
    return 1;
}

/* ═══════════════════════════════════════════════
   LIBERATION
═══════════════════════════════════════════════ */
void Liberation(Game *game, SDL_Window *window, SDL_Renderer *renderer) {
    /* Project-1 assets */
    if (game->music)        Mix_FreeMusic(game->music);
    if (game->Sound)        Mix_FreeChunk(game->Sound);
    if (game->font)         TTF_CloseFont(game->font);
    if (game->background)   SDL_DestroyTexture(game->background);
    if (game->logoTexture)  SDL_DestroyTexture(game->logoTexture);
    if (game->titleTexture) SDL_DestroyTexture(game->titleTexture);
    if (game->trapTexture)  SDL_DestroyTexture(game->trapTexture);
    for (int i = 0; i < 5; i++) {
        if (game->buttons[i].normalTex) SDL_DestroyTexture(game->buttons[i].normalTex);
        if (game->buttons[i].hoverTex)  SDL_DestroyTexture(game->buttons[i].hoverTex);
    }

    /* Project-2 assets */
    if (game->msGreTex)  SDL_DestroyTexture(game->msGreTex);
    if (game->msRedTex)  SDL_DestroyTexture(game->msRedTex);
    if (game->leaderTex) SDL_DestroyTexture(game->leaderTex);
    if (game->click)     Mix_FreeChunk(game->click);

    /* Game state assets */
    for (int i = 0; i < 5; i++) {
        if (game->gameButtons[i].normalTex) SDL_DestroyTexture(game->gameButtons[i].normalTex);
        if (game->gameButtons[i].hoverTex)  SDL_DestroyTexture(game->gameButtons[i].hoverTex);
    }

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}

/* ═══════════════════════════════════════════════
   MENU  –  INPUT
═══════════════════════════════════════════════ */
void Menu_LectureEntree(Game *game) {
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

/* ═══════════════════════════════════════════════
   MENU  –  RENDER
═══════════════════════════════════════════════ */
void Menu_Affichage(Game *game, SDL_Renderer *renderer) {
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

/* ═══════════════════════════════════════════════
   LEADERBOARD  –  INPUT
═══════════════════════════════════════════════ */
void Leaderboard_LectureEntree(Game *game) {
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

/* ═══════════════════════════════════════════════
   LEADERBOARD  –  RENDER
═══════════════════════════════════════════════ */
void Leaderboard_Affichage(Game *game, SDL_Renderer *renderer) {
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
/* ═══════════════════════════════════════════════
   GAME  –  INPUT
═══════════════════════════════════════════════ */
void Jeu_LectureEntree(Game *game) {
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

/* ═══════════════════════════════════════════════
   GAME  –  RENDER
═══════════════════════════════════════════════ */
void Jeu_Affichage(Game *game, SDL_Renderer *renderer) {
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

/* ═══════════════════════════════════════════════
   GAME BUTTON CLICK HANDLER
═══════════════════════════════════════════════ */
void HandleGameButtonClick(Game *game) {
    /* Play click sound effect */
    if (game->click)
        Mix_PlayChannel(-1, game->click, 0);
    
    /* Transition from game state to score state */
    game->currentState = STATE_SCORES;
}