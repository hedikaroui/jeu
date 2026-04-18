#include "game.h"
#include <stdio.h>

static OptionsUiState optionsUi = {.pendingCommand = OPTIONS_COMMAND_NONE};

static int create_text_texture(SDL_Renderer *renderer, const char *font_path, int pt_size,
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

static SDL_Texture *load_texture_first(SDL_Renderer *renderer, const char *a, const char *b, const char *c) {
    SDL_Texture *t = NULL;
    if (a) t = IMG_LoadTexture(renderer, a);
    if (!t && b) t = IMG_LoadTexture(renderer, b);
    if (!t && c) t = IMG_LoadTexture(renderer, c);
    return t;
}

static int options_mouse_over(SDL_Rect r, int x, int y) {
    return (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h);
}

static void options_draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
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

static void options_sync_layout(Game *game) {
    int w = WIDTH;
    int h = HEIGHT;
    int bw = 80;
    int bh = 80;
    int center_x;
    int button_y;

    if (game->window) SDL_GetWindowSize(game->window, &w, &h);

    center_x = w / 2;
    button_y = h / 2;
    optionsUi.backgroundRect = (SDL_Rect){0, 0, w, h};
    optionsUi.minusRect = (SDL_Rect){center_x - 180, button_y, bw, bh};
    optionsUi.muteRect = (SDL_Rect){center_x - 40, button_y, bw, bh};
    optionsUi.plusRect = (SDL_Rect){center_x + 100, button_y, bw, bh};
    optionsUi.fullscreenRect = (SDL_Rect){center_x - 40, button_y + 120, bw, bh};

    if (game->optionsTitle) {
        game->optionsTitleRect.x = (w - game->optionsTitleRect.w) / 2;
        game->optionsTitleRect.y = 40;
    }
}

static OptionsCommand options_command_from_point(int x, int y) {
    if (options_mouse_over(optionsUi.minusRect, x, y)) return OPTIONS_COMMAND_VOLUME_DOWN;
    if (options_mouse_over(optionsUi.muteRect, x, y)) return OPTIONS_COMMAND_VOLUME_MUTE;
    if (options_mouse_over(optionsUi.plusRect, x, y)) return OPTIONS_COMMAND_VOLUME_UP;
    if (options_mouse_over(optionsUi.fullscreenRect, x, y)) return OPTIONS_COMMAND_FULLSCREEN;
    return OPTIONS_COMMAND_NONE;
}

int Options_Charger(Game *game, SDL_Renderer *renderer) {
    SDL_Color white = {255, 255, 255, 255};

    if (game->optionsLoaded) return 1;

    game->optionsBg = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.options);
    game->volumePlusBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_PLUS);
    game->volumeMinusBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_MINUS);
    game->volumeMuteBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_MUTE);
    game->volumeMinusHoverBtn = load_texture_first(renderer, OPTION_BUTTON_VOLUME_MINUS_HOVER, OPTION_BUTTON_VOLUME_MINUS_HOVER, NULL);
    game->volumePlusHoverBtn = load_texture_first(renderer, OPTION_BUTTON_VOLUME_PLUS_HOVER_1, OPTION_BUTTON_VOLUME_PLUS_HOVER_2, NULL);
    game->volumeMuteHoverBtn = load_texture_first(renderer, OPTION_BUTTON_VOLUME_MUTE_HOVER_1, OPTION_BUTTON_VOLUME_MUTE_HOVER_2, NULL);
    game->fullscreenBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_FULLSCREEN);
    game->normalscreenBtn = IMG_LoadTexture(renderer, OPTION_BUTTON_NORMALSCREEN);
    game->optionsClick = Mix_LoadWAV(SOUND_OPTION_CLICK);
    game->optionsTitle = NULL;
    game->optionsTitleRect = (SDL_Rect){0, 40, 0, 0};

    if (!create_text_texture(renderer, GAME_ASSETS.fonts.system_bold, 60,
                             "OPTIONS", white,
                             &game->optionsTitle, &game->optionsTitleRect)) {
        create_text_texture(renderer, GAME_ASSETS.fonts.system_regular, 60,
                            "OPTIONS", white,
                            &game->optionsTitle, &game->optionsTitleRect);
    }

    game->optionsFullscreen = game->fullscreen;
    game->optionsLoaded = 1;
    optionsUi.pendingCommand = OPTIONS_COMMAND_NONE;
    options_sync_layout(game);
    printf("Options: assets charges\n");
    return 1;
}

void Options_LectureEntree(Game *game) {
    SDL_Event e;
    options_sync_layout(game);
    optionsUi.pendingCommand = OPTIONS_COMMAND_NONE;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE: optionsUi.pendingCommand = OPTIONS_COMMAND_BACK; return;
                case SDLK_MINUS:
                case SDLK_UNDERSCORE: optionsUi.pendingCommand = OPTIONS_COMMAND_VOLUME_DOWN; return;
                case SDLK_PLUS:
                case SDLK_EQUALS: optionsUi.pendingCommand = OPTIONS_COMMAND_VOLUME_UP; return;
                case SDLK_m: optionsUi.pendingCommand = OPTIONS_COMMAND_VOLUME_MUTE; return;
                case SDLK_f: optionsUi.pendingCommand = OPTIONS_COMMAND_FULLSCREEN; return;
                default: break;
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            optionsUi.pendingCommand = options_command_from_point(e.button.x, e.button.y);
            if (optionsUi.pendingCommand != OPTIONS_COMMAND_NONE) return;
        }
    }
}

void Options_Affichage(Game *game, SDL_Renderer *renderer) {
    char volume_label[32];
    SDL_Rect volume_box;

    options_sync_layout(game);

    if (game->optionsBg)
        SDL_RenderCopy(renderer, game->optionsBg, NULL, &optionsUi.backgroundRect);
    if (game->optionsTitle)
        SDL_RenderCopy(renderer, game->optionsTitle, NULL, &game->optionsTitleRect);

    if (optionsUi.hoverMinus && game->volumeMinusHoverBtn)
        SDL_RenderCopy(renderer, game->volumeMinusHoverBtn, NULL, &optionsUi.minusRect);
    else if (game->volumeMinusBtn)
        SDL_RenderCopy(renderer, game->volumeMinusBtn, NULL, &optionsUi.minusRect);

    if (optionsUi.hoverMute && game->volumeMuteHoverBtn)
        SDL_RenderCopy(renderer, game->volumeMuteHoverBtn, NULL, &optionsUi.muteRect);
    else if (game->volumeMuteBtn)
        SDL_RenderCopy(renderer, game->volumeMuteBtn, NULL, &optionsUi.muteRect);

    if (optionsUi.hoverPlus && game->volumePlusHoverBtn)
        SDL_RenderCopy(renderer, game->volumePlusHoverBtn, NULL, &optionsUi.plusRect);
    else if (game->volumePlusBtn)
        SDL_RenderCopy(renderer, game->volumePlusBtn, NULL, &optionsUi.plusRect);

    if (game->optionsFullscreen && game->normalscreenBtn)
        SDL_RenderCopy(renderer, game->normalscreenBtn, NULL, &optionsUi.fullscreenRect);
    else if (game->fullscreenBtn)
        SDL_RenderCopy(renderer, game->fullscreenBtn, NULL, &optionsUi.fullscreenRect);

    volume_box = (SDL_Rect){(optionsUi.backgroundRect.w - 240) / 2, optionsUi.plusRect.y - 86, 240, 46};
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 165);
    SDL_RenderFillRect(renderer, &volume_box);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &volume_box);
    snprintf(volume_label, sizeof(volume_label), "Volume : %d / 128", game->volume);
    options_draw_center_text(renderer, game->font, volume_label, (SDL_Color){255, 255, 255, 255}, volume_box);
}

void Options_MiseAJour(Game *game) {
    int mx = 0;
    int my = 0;
    int new_volume;

    options_sync_layout(game);
    SDL_GetMouseState(&mx, &my);
    optionsUi.hoverMinus = options_mouse_over(optionsUi.minusRect, mx, my);
    optionsUi.hoverMute = options_mouse_over(optionsUi.muteRect, mx, my);
    optionsUi.hoverPlus = options_mouse_over(optionsUi.plusRect, mx, my);
    optionsUi.hoverFullscreen = options_mouse_over(optionsUi.fullscreenRect, mx, my);

    switch (optionsUi.pendingCommand) {
        case OPTIONS_COMMAND_BACK:
            Game_SetSubState(game, STATE_MENU);
            if (game->music) Mix_PlayMusic(game->music, -1);
            break;
        case OPTIONS_COMMAND_VOLUME_DOWN:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            new_volume = Mix_VolumeMusic(-1) - 10;
            if (new_volume < 0) new_volume = 0;
            Mix_VolumeMusic(new_volume);
            game->volume = new_volume;
            break;
        case OPTIONS_COMMAND_VOLUME_MUTE:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            Mix_VolumeMusic(0);
            game->volume = 0;
            break;
        case OPTIONS_COMMAND_VOLUME_UP:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            new_volume = Mix_VolumeMusic(-1) + 10;
            if (new_volume > 128) new_volume = 128;
            Mix_VolumeMusic(new_volume);
            game->volume = new_volume;
            break;
        case OPTIONS_COMMAND_FULLSCREEN:
            if (game->optionsClick) Mix_PlayChannel(-1, game->optionsClick, 0);
            game->optionsFullscreen = !game->optionsFullscreen;
            game->fullscreen = game->optionsFullscreen;
            if (game->window) {
                Uint32 flags = game->optionsFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                SDL_SetWindowFullscreen(game->window, flags);
            }
            break;
        case OPTIONS_COMMAND_NONE:
        default:
            break;
    }

    optionsUi.pendingCommand = OPTIONS_COMMAND_NONE;
    SDL_Delay(16);
}
