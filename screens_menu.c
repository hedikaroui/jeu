#include "game.h"
#include <stdio.h>
#include <string.h>

#define OPTIONS_CMD_NONE 0
#define OPTIONS_CMD_BACK 1
#define OPTIONS_CMD_VOLUME_DOWN 2
#define OPTIONS_CMD_VOLUME_MUTE 3
#define OPTIONS_CMD_VOLUME_UP 4
#define OPTIONS_CMD_FULLSCREEN 5

typedef struct {
    SDL_Rect backgroundRect;
    SDL_Rect minusRect;
    SDL_Rect muteRect;
    SDL_Rect plusRect;
    SDL_Rect fullscreenRect;
    int hoverMinus;
    int hoverMute;
    int hoverPlus;
    int hoverFullscreen;
    int pendingCommand;
} ScreenOptionsUiState;

ScreenOptionsUiState screensOptionsUi = {.pendingCommand = OPTIONS_CMD_NONE};

int screens_create_text_texture(SDL_Renderer *renderer, const char *font_path, int pt_size,
                                const char *text, SDL_Color color,
                                SDL_Texture **texture, SDL_Rect *rect) {
    TTF_Font *font = NULL;
    SDL_Surface *surface = NULL;
    if (!renderer || !font_path || !text || !texture || !rect) return 0;
    font = TTF_OpenFont(font_path, pt_size);
    if (!font) return 0;
    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) { TTF_CloseFont(font); return 0; }
    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (*texture) {
        rect->x = 0; rect->y = 0; rect->w = surface->w; rect->h = surface->h;
    }
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
    return (*texture != NULL);
}

SDL_Texture *screens_load_texture_first(SDL_Renderer *renderer, const char *a, const char *b, const char *c) {
    SDL_Texture *t = NULL;
    if (a) t = IMG_LoadTexture(renderer, a);
    if (!t && b) t = IMG_LoadTexture(renderer, b);
    if (!t && c) t = IMG_LoadTexture(renderer, c);
    return t;
}

int screens_options_mouse_over(SDL_Rect r, int x, int y) {
    return (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h);
}

void screens_options_draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                                      SDL_Color color, SDL_Rect box) {
    if (!font || !text || !*text) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        SDL_Rect dst = {box.x + (box.w - surf->w) / 2, box.y + (box.h - surf->h) / 2, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void screens_options_sync_layout(Game *game) {
    int w = WIDTH;
    int h = HEIGHT;
    int bw = 80;
    int bh = 80;
    int center_x;
    int button_y;

    if (game->window) SDL_GetWindowSize(game->window, &w, &h);

    center_x = w / 2;
    button_y = h / 2;
    screensOptionsUi.backgroundRect = (SDL_Rect){0, 0, w, h};
    screensOptionsUi.minusRect = (SDL_Rect){center_x - 180, button_y, bw, bh};
    screensOptionsUi.muteRect = (SDL_Rect){center_x - 40, button_y, bw, bh};
    screensOptionsUi.plusRect = (SDL_Rect){center_x + 100, button_y, bw, bh};
    screensOptionsUi.fullscreenRect = (SDL_Rect){center_x - 40, button_y + 120, bw, bh};

    if (game->optionsTitle) {
        game->optionsTitleRect.x = (w - game->optionsTitleRect.w) / 2;
        game->optionsTitleRect.y = 40;
    }
}

int screens_options_command_from_point(int x, int y) {
    if (screens_options_mouse_over(screensOptionsUi.minusRect, x, y)) return OPTIONS_CMD_VOLUME_DOWN;
    if (screens_options_mouse_over(screensOptionsUi.muteRect, x, y)) return OPTIONS_CMD_VOLUME_MUTE;
    if (screens_options_mouse_over(screensOptionsUi.plusRect, x, y)) return OPTIONS_CMD_VOLUME_UP;
    if (screens_options_mouse_over(screensOptionsUi.fullscreenRect, x, y)) return OPTIONS_CMD_FULLSCREEN;
    return OPTIONS_CMD_NONE;
}

int Options_Charger(Game *game, SDL_Renderer *renderer) {
    SDL_Color white = {255, 255, 255, 255};

    if (game->optionsLoaded) return 1;

    game->optionsBg = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.options);
    game->volumePlusBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_PLUS);
    game->volumeMinusBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_MINUS);
    game->volumeMuteBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_MUTE);
    game->volumeMinusHoverBtn = screens_load_texture_first(renderer, OPTION_BUTTON_VOLUME_MINUS_HOVER, OPTION_BUTTON_VOLUME_MINUS_HOVER, NULL);
    game->volumePlusHoverBtn = screens_load_texture_first(renderer, OPTION_BUTTON_VOLUME_PLUS_HOVER_1, OPTION_BUTTON_VOLUME_PLUS_HOVER_2, NULL);
    game->volumeMuteHoverBtn = screens_load_texture_first(renderer, OPTION_BUTTON_VOLUME_MUTE_HOVER_1, OPTION_BUTTON_VOLUME_MUTE_HOVER_2, NULL);
    game->fullscreenBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_FULLSCREEN);
    game->normalscreenBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_NORMALSCREEN);
    game->optionsClick = Mix_LoadWAV(SOUND_OPTION_CLICK);
    game->optionsTitle = NULL;
    game->optionsTitleRect = (SDL_Rect){0, 40, 0, 0};

    if (!screens_create_text_texture(renderer, GAME_ASSETS.fonts.system_bold, 60,
                                     "OPTIONS", white,
                                     &game->optionsTitle, &game->optionsTitleRect)) {
        screens_create_text_texture(renderer, GAME_ASSETS.fonts.system_regular, 60,
                                    "OPTIONS", white,
                                    &game->optionsTitle, &game->optionsTitleRect);
    }

    game->optionsFullscreen = game->fullscreen;
    game->optionsLoaded = 1;
    screensOptionsUi.pendingCommand = OPTIONS_CMD_NONE;
    screens_options_sync_layout(game);
    printf("Options: assets charges\n");
    return 1;
}

void Options_LectureEntree(Game *game) {
    SDL_Event e;
    screens_options_sync_layout(game);
    screensOptionsUi.pendingCommand = OPTIONS_CMD_NONE;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE: screensOptionsUi.pendingCommand = OPTIONS_CMD_BACK; return;
                case SDLK_MINUS:
                case SDLK_UNDERSCORE: screensOptionsUi.pendingCommand = OPTIONS_CMD_VOLUME_DOWN; return;
                case SDLK_PLUS:
                case SDLK_EQUALS: screensOptionsUi.pendingCommand = OPTIONS_CMD_VOLUME_UP; return;
                case SDLK_m: screensOptionsUi.pendingCommand = OPTIONS_CMD_VOLUME_MUTE; return;
                case SDLK_f: screensOptionsUi.pendingCommand = OPTIONS_CMD_FULLSCREEN; return;
                default: break;
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            screensOptionsUi.pendingCommand = screens_options_command_from_point(e.button.x, e.button.y);
            if (screensOptionsUi.pendingCommand != OPTIONS_CMD_NONE) return;
        }
    }
}

void Options_Affichage(Game *game, SDL_Renderer *renderer) {
    char volume_label[32];
    SDL_Rect volume_box;

    screens_options_sync_layout(game);

    if (game->optionsBg)
        SDL_RenderCopy(renderer, game->optionsBg, NULL, &screensOptionsUi.backgroundRect);
    if (game->optionsTitle)
        SDL_RenderCopy(renderer, game->optionsTitle, NULL, &game->optionsTitleRect);

    if (screensOptionsUi.hoverMinus && game->volumeMinusHoverBtn)
        SDL_RenderCopy(renderer, game->volumeMinusHoverBtn, NULL, &screensOptionsUi.minusRect);
    else if (game->volumeMinusBtn)
        SDL_RenderCopy(renderer, game->volumeMinusBtn, NULL, &screensOptionsUi.minusRect);

    if (screensOptionsUi.hoverMute && game->volumeMuteHoverBtn)
        SDL_RenderCopy(renderer, game->volumeMuteHoverBtn, NULL, &screensOptionsUi.muteRect);
    else if (game->volumeMuteBtn)
        SDL_RenderCopy(renderer, game->volumeMuteBtn, NULL, &screensOptionsUi.muteRect);

    if (screensOptionsUi.hoverPlus && game->volumePlusHoverBtn)
        SDL_RenderCopy(renderer, game->volumePlusHoverBtn, NULL, &screensOptionsUi.plusRect);
    else if (game->volumePlusBtn)
        SDL_RenderCopy(renderer, game->volumePlusBtn, NULL, &screensOptionsUi.plusRect);

    if (game->optionsFullscreen && game->normalscreenBtn)
        SDL_RenderCopy(renderer, game->normalscreenBtn, NULL, &screensOptionsUi.fullscreenRect);
    else if (game->fullscreenBtn)
        SDL_RenderCopy(renderer, game->fullscreenBtn, NULL, &screensOptionsUi.fullscreenRect);

    volume_box = (SDL_Rect){(screensOptionsUi.backgroundRect.w - 240) / 2, screensOptionsUi.plusRect.y - 86, 240, 46};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 165);
    SDL_RenderFillRect(renderer, &volume_box);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &volume_box);
    snprintf(volume_label, sizeof(volume_label), "Volume : %d / 128", game->volume);
    screens_options_draw_center_text(renderer, game->font, volume_label, (SDL_Color){255, 255, 255, 255}, volume_box);
}

void Options_MiseAJour(Game *game) {
    int mx = 0;
    int my = 0;
    int new_volume;

    screens_options_sync_layout(game);
    SDL_GetMouseState(&mx, &my);
    screensOptionsUi.hoverMinus = screens_options_mouse_over(screensOptionsUi.minusRect, mx, my);
    screensOptionsUi.hoverMute = screens_options_mouse_over(screensOptionsUi.muteRect, mx, my);
    screensOptionsUi.hoverPlus = screens_options_mouse_over(screensOptionsUi.plusRect, mx, my);
    screensOptionsUi.hoverFullscreen = screens_options_mouse_over(screensOptionsUi.fullscreenRect, mx, my);

    switch (screensOptionsUi.pendingCommand) {
        case OPTIONS_CMD_BACK:
            Game_SetSubState(game, STATE_MENU);
            if (game->music) Mix_PlayMusic(game->music, -1);
            break;
        case OPTIONS_CMD_VOLUME_DOWN:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            new_volume = Mix_VolumeMusic(-1) - 10;
            if (new_volume < 0) new_volume = 0;
            Mix_VolumeMusic(new_volume);
            game->volume = new_volume;
            break;
        case OPTIONS_CMD_VOLUME_MUTE:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            Mix_VolumeMusic(0);
            game->volume = 0;
            break;
        case OPTIONS_CMD_VOLUME_UP:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            new_volume = Mix_VolumeMusic(-1) + 10;
            if (new_volume > 128) new_volume = 128;
            Mix_VolumeMusic(new_volume);
            game->volume = new_volume;
            break;
        case OPTIONS_CMD_FULLSCREEN:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            game->optionsFullscreen = !game->optionsFullscreen;
            game->fullscreen = game->optionsFullscreen;
            if (game->window) {
                Uint32 flags = game->optionsFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                SDL_SetWindowFullscreen(game->window, flags);
            }
            break;
        case OPTIONS_CMD_NONE:
        default:
            break;
    }

    screensOptionsUi.pendingCommand = OPTIONS_CMD_NONE;
    SDL_Delay(16);
}

int Save_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->saveBg) return 1;

    game->saveBg = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.save_primary);
    if (!game->saveBg)
        game->saveBg = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.save_fallback);
    if (!game->saveBg) { printf("Erreur BG.png/LB.jpg: %s\n", IMG_GetError()); return 0; }

    game->saveMusic = Mix_LoadMUS(GAME_ASSETS.songs.save_music_primary);
    if (!game->saveMusic)
        game->saveMusic = Mix_LoadMUS(GAME_ASSETS.songs.save_music_fallback);
    if (game->saveMusic) Mix_PlayMusic(game->saveMusic, -1);
    game->saveSound = Mix_LoadWAV(SOUND_SAVE_CLICK);

    TTF_Font *f = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 36);
    if (!f) f = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 36);
    if (f) {
        SDL_Color white = {255,255,255,255};
        SDL_Color red = {178,34,34,255};
        game->titleSurface = TTF_RenderUTF8_Shaded(f,
                             "Voulez-vous sauvegarder la partie ?", white, red);
        TTF_CloseFont(f);
    }
    if (game->titleSurface) {
        game->titleTexSave  = SDL_CreateTextureFromSurface(renderer, game->titleSurface);
        game->titleRectSave = (SDL_Rect){
            (WIDTH  - game->titleSurface->w) / 2, 30,
             game->titleSurface->w, game->titleSurface->h
        };
    }

    game->saveButtons[0].rect        = (SDL_Rect){200, 300, 380, 200};
    game->saveButtons[0].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[0]);
    game->saveButtons[0].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[0]);
    game->saveButtons[1].rect        = (SDL_Rect){700, 300, 380, 200};
    game->saveButtons[1].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[1]);
    game->saveButtons[1].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[1]);
    game->saveButtons[2].rect        = (SDL_Rect){240, 220, 420, 180};
    game->saveButtons[2].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[2]);
    game->saveButtons[2].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[2]);
    game->saveButtons[3].rect        = (SDL_Rect){620, 220, 420, 180};
    game->saveButtons[3].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[3]);
    game->saveButtons[3].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[3]);

    game->saveEtat = 0;
    game->clic_bouton = -1;
    return 1;
}

void Save_LectureEntree(Game *game) {
    SDL_Event event;
    game->clic_bouton = -1;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { game->running = 0; return; }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            Game_SetSubState(game, STATE_MENU);
            if (game->music) Mix_PlayMusic(game->music, -1);
            return;
        }
        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x, my = event.motion.y;
            for (int i = 0; i < 4; i++) game->saveButtons[i].selected = 0;
            if (game->saveEtat == 0) {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[0].rect)) game->saveButtons[0].selected = 1;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[1].rect)) game->saveButtons[1].selected = 1;
            } else {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[2].rect)) game->saveButtons[2].selected = 1;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[3].rect)) game->saveButtons[3].selected = 1;
            }
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;
            if (game->saveEtat == 0) {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[0].rect)) game->clic_bouton = 0;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[1].rect)) game->clic_bouton = 1;
            } else {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[2].rect)) game->clic_bouton = 2;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[3].rect)) game->clic_bouton = 3;
            }
        }
    }
}

void Save_Affichage(Game *game, SDL_Renderer *renderer) {
    if (game->saveBg) SDL_RenderCopy(renderer, game->saveBg, NULL, NULL);
    if (game->saveEtat == 0) {
        if (game->titleTexSave)
            SDL_RenderCopy(renderer, game->titleTexSave, NULL, &game->titleRectSave);
        for (int i = 0; i < 2; i++) {
            SDL_Texture *t = game->saveButtons[i].selected
                             ? game->saveButtons[i].textureCliq
                             : game->saveButtons[i].texture;
            if (t) SDL_RenderCopy(renderer, t, NULL, &game->saveButtons[i].rect);
        }
    } else {
        for (int i = 2; i < 4; i++) {
            SDL_Texture *t = game->saveButtons[i].selected
                             ? game->saveButtons[i].textureCliq
                             : game->saveButtons[i].texture;
            if (t) SDL_RenderCopy(renderer, t, NULL, &game->saveButtons[i].rect);
        }
    }
}

void Save_MiseAJour(Game *game) {
    if (game->clic_bouton == 0) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        game->saveEtat = 1;
        Game_SetSubState(game, STATE_SAVE_CHOICE);
    }
    if (game->clic_bouton == 1) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        game->saveEtat = 0; Game_SetSubState(game, STATE_MENU);
        if (game->music) Mix_PlayMusic(game->music, -1);
    }
    if (game->clic_bouton == 2) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        printf("Chargement sauvegarde\n");
        game->saveEtat = 0;
        Game_SetSubState(game, STATE_PLAYER);
    }
    if (game->clic_bouton == 3) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        printf("Nouvelle partie\n");
        game->saveEtat = 0;
        Game_SetSubState(game, STATE_PLAYER);
    }
    SDL_Delay(16);
}

SDL_Rect leaderboardStartBtnRect = {0, 0, 0, 0};
int leaderboardStartBtnHover = 0;
Uint32 leaderboardCursorLastBlink = 0;
int leaderboardCursorOn = 1;

void screens_leaderboard_sync_layout(Game *game) {
    int w = WIDTH;
    int h = HEIGHT;
    if (game->window) SDL_GetWindowSize(game->window, &w, &h);
    game->searchBox = (SDL_Rect){(w - 400) / 2, 140, 400, 50};
    leaderboardStartBtnRect = (SDL_Rect){(w - 320) / 2, h - 130, 320, 90};
}

void Leaderboard_LectureEntree(Game *game) {
    SDL_Event e;
    screens_leaderboard_sync_layout(game);
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                game->inputActive = 0;
                SDL_StopTextInput();
                if (game->currentSubState == STATE_SCORES_LIST) {
                    Game_SetSubState(game, STATE_QUIT);
                } else {
                    Game_SetSubState(game, STATE_MENU);
                    if (game->music) Mix_PlayMusic(game->music, -1);
                }
                return;
            }
            if (e.key.keysym.sym == SDLK_e) {
                Game_SetSubState(game, STATE_ENIGME_QUIZ);
                return;
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->backBtn.rect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                Game_SetSubState(game, STATE_MENU);
                game->inputActive = 0;
                SDL_StopTextInput();
                if (game->music) Mix_PlayMusic(game->music, -1);
                return;
            }
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &leaderboardStartBtnRect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                Game_SetSubState(game, STATE_START_PLAY);
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
            leaderboardStartBtnHover = SDL_PointInRect(&(SDL_Point){e.motion.x, e.motion.y}, &leaderboardStartBtnRect);
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
                Game_SetSubState(game, STATE_SCORES_LIST);
                return;
            }
        }
    }
}

void Leaderboard_Affichage(Game *game, SDL_Renderer *renderer) {
    screens_leaderboard_sync_layout(game);

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
        Uint32 now = SDL_GetTicks();
        if (now - leaderboardCursorLastBlink > 500) {
            leaderboardCursorOn = !leaderboardCursorOn;
            leaderboardCursorLastBlink = now;
        }
        if (game->inputActive && leaderboardCursorOn) {
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
        SDL_Rect drawRect = leaderboardStartBtnRect;
        SDL_Texture *t = leaderboardStartBtnHover ? game->scoreJ2Tex : game->scoreJ1Tex;
        if (t) {
            if (leaderboardStartBtnHover) drawRect.y -= 4;
            SDL_RenderCopy(renderer, t, NULL, &drawRect);
        }
    }
}
