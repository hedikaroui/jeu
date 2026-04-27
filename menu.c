#include "game.h"
#include <math.h>
#include <string.h>

MenuUiState menuUi = {
    .hoveredButton = -1,
    .clickedButton = -1
};

int menu_create_text_texture(SDL_Renderer *renderer, const char *font_path, int pt_size,
                               const char *text, SDL_Color color,
                               SDL_Texture **texture, SDL_Rect *rect) {
    TTF_Font *font = NULL;
    SDL_Surface *surface = NULL;

    if (!renderer || !font_path || !text || !texture || !rect) return 0;

    font = TTF_OpenFont(font_path, pt_size);
    if (!font) return 0;

    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        TTF_CloseFont(font);
        return 0;
    }

    *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (*texture) {
        rect->x = 0;
        rect->y = 0;
        rect->w = surface->w;
        rect->h = surface->h;
    }

    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
    return (*texture != NULL);
}

SDL_Rect menu_gift_rect(int w, int h, Uint32 ticks) {
    int shakeX = (int)(3.0 * sin((double)ticks / 55.0));
    int shakeY = (int)(2.0 * cos((double)ticks / 70.0));
    return (SDL_Rect){w - 170 + shakeX, h - 170 + shakeY, 140, 140};
}

void menu_init_background_assets(Game *game, SDL_Renderer *renderer) {
    SDL_Color title_color = {245, 245, 245, 255};

    game->background = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.menu);
    game->logoTexture = IMG_LoadTexture(renderer, GAME_ASSETS.characters.logo);
    game->titleTexture = NULL;
    game->trapTexture = NULL;
    game->titleRect = (SDL_Rect){0, 0, 0, 0};
    game->trapRect = (SDL_Rect){0, 0, 0, 0};

    if (!menu_create_text_texture(renderer, GAME_ASSETS.fonts.system_bold, 54,
                             "MENU PRINCIPAL", title_color,
                             &game->titleTexture, &game->titleRect)) {
        menu_create_text_texture(renderer, GAME_ASSETS.fonts.system_regular, 54,
                            "MENU PRINCIPAL", title_color,
                            &game->titleTexture, &game->titleRect);
    }
}

void menu_init_button_assets(Game *game, SDL_Renderer *renderer) {
    for (int i = 0; i < 5; i++) {
        game->buttons[i].normalTex = IMG_LoadTexture(renderer, MENU_BUTTON_NORMAL[i]);
        game->buttons[i].hoverTex = IMG_LoadTexture(renderer, MENU_BUTTON_HOVER[i]);
        game->buttons[i].rect = (SDL_Rect){0, 0, 0, 0};
        game->buttons[i].selected = 0;
    }
}

void menu_sync_layout(Game *game) {
    int w = WIDTH;
    int h = HEIGHT;
    int bw = 430;
    int bh = 120;
    int gap = 20;
    int total_height;
    int bx = 48;
    int by0;

    if (game->window) SDL_GetWindowSize(game->window, &w, &h);

    if (w < 1150) {
        bw = 360;
        bh = 96;
        gap = 16;
    }
    if (w < 920) {
        bw = 300;
        bh = 84;
        gap = 12;
    }

    menuUi.backgroundRect = (SDL_Rect){0, 0, w, h};

    if (game->logoTexture) {
        int logo_size = (w < 1024) ? 220 : 300;
        game->logoRect = (SDL_Rect){w - logo_size - 24, 20, logo_size, logo_size};
    }

    if (game->titleTexture) {
        int tw = game->titleRect.w;
        int th = game->titleRect.h;
        game->titleRect = (SDL_Rect){48, 40, tw, th};
    }

    total_height = 5 * bh + 4 * gap;
    by0 = (h - total_height) / 2;
    if (by0 < 160) by0 = 160;

    for (int i = 0; i < 5; i++) {
        game->buttons[i].rect = (SDL_Rect){bx, by0 + i * (bh + gap), bw, bh};
    }

    menuUi.giftRect = menu_gift_rect(w, h, SDL_GetTicks());
}

MenuCommand menu_command_from_button(int button_index) {
    switch (button_index) {
        case 0: return MENU_COMMAND_PLAY;
        case 1: return MENU_COMMAND_OPTIONS;
        case 2: return MENU_COMMAND_SAVE;
        case 3: return MENU_COMMAND_SCORES;
        case 4: return MENU_COMMAND_QUIT;
        default: return MENU_COMMAND_NONE;
    }
}

void Menu_Preparer(Game *game, SDL_Renderer *renderer) {
    menu_init_background_assets(game, renderer);
    menu_init_button_assets(game, renderer);
    menuUi.hoveredButton = -1;
    menuUi.clickedButton = -1;
    menuUi.pendingCommand = MENU_COMMAND_NONE;
    menuUi.hoverGift = 0;
    menuUi.clickGift = 0;
    menuUi.hoverSoundPending = 0;
    menu_sync_layout(game);
}

void Menu_LectureEntree(Game *game) {
    SDL_Event event;
    menu_sync_layout(game);
    menuUi.clickedButton = -1;
    menuUi.clickGift = 0;
    menuUi.pendingCommand = MENU_COMMAND_NONE;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            menuUi.pendingCommand = MENU_COMMAND_QUIT;
            return;
        }

        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    menuUi.pendingCommand = MENU_COMMAND_QUIT;
                    return;
                case SDLK_j:
                    menuUi.pendingCommand = MENU_COMMAND_PLAY;
                    return;
                case SDLK_o:
                    menuUi.pendingCommand = MENU_COMMAND_OPTIONS;
                    return;
                case SDLK_s:
                    menuUi.pendingCommand = MENU_COMMAND_SAVE;
                    return;
                case SDLK_m:
                    menuUi.pendingCommand = MENU_COMMAND_SCORES;
                    return;
                default:
                    break;
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int mx = event.button.x, my = event.button.y;
            if (SDL_PointInRect(&(SDL_Point){mx, my}, &menuUi.giftRect)) {
                menuUi.clickGift = 1;
                return;
            }

            for (int i = 0; i < 5; i++) {
                if (SDL_PointInRect(&(SDL_Point){mx, my}, &game->buttons[i].rect)) {
                    menuUi.clickedButton = i;
                    return;
                }
            }
        }
    }
}

void Menu_MiseAJour(Game *game) {
    int mx = 0;
    int my = 0;
    int previous_hover = menuUi.hoveredButton;
    int new_hover = -1;

    menu_sync_layout(game);
    SDL_GetMouseState(&mx, &my);

    for (int i = 0; i < 5; i++) {
        int is_hovered = SDL_PointInRect(&(SDL_Point){mx, my}, &game->buttons[i].rect);
        game->buttons[i].selected = is_hovered;
        if (is_hovered && new_hover == -1) new_hover = i;
    }

    menuUi.hoveredButton = new_hover;
    menuUi.hoverGift = SDL_PointInRect(&(SDL_Point){mx, my}, &menuUi.giftRect);
    if (menuUi.hoveredButton != previous_hover && menuUi.hoveredButton >= 0) {
        menuUi.hoverSoundPending = 1;
    }
    if (menuUi.hoverSoundPending && game->Sound) {
        Mix_PlayChannel(-1, game->Sound, 0);
        menuUi.hoverSoundPending = 0;
    }

    if (menuUi.clickGift) {
        menuUi.pendingCommand = MENU_COMMAND_GIFT;
    } else if (menuUi.clickedButton >= 0) {
        menuUi.pendingCommand = menu_command_from_button(menuUi.clickedButton);
    }

    switch (menuUi.pendingCommand) {
        case MENU_COMMAND_PLAY:
            if (game->click) Mix_PlayChannel(-1, game->click, 0);
            Game_SetSubState(game, STATE_SAVE);
            break;
        case MENU_COMMAND_OPTIONS:
            if (game->click) Mix_PlayChannel(-1, game->click, 0);
            Game_SetSubState(game, STATE_OPTIONS);
            break;
        case MENU_COMMAND_SAVE:
            if (game->click) Mix_PlayChannel(-1, game->click, 0);
            Game_SetSubState(game, STATE_SAVE);
            break;
        case MENU_COMMAND_SCORES:
            if (game->click) Mix_PlayChannel(-1, game->click, 0);
            Game_SetSubState(game, STATE_SCORES_INPUT);
            break;
        case MENU_COMMAND_GIFT:
            if (game->click) Mix_PlayChannel(-1, game->click, 0);
            Game_SetSubState(game, STATE_ENIGME_QUIZ);
            break;
        case MENU_COMMAND_QUIT:
            Game_SetSubState(game, STATE_QUIT);
            break;
        case MENU_COMMAND_NONE:
        default:
            break;
    }

    menuUi.clickedButton = -1;
    menuUi.clickGift = 0;
    menuUi.pendingCommand = MENU_COMMAND_NONE;
}

void Menu_Affichage(Game *game, SDL_Renderer *renderer) {
    menu_sync_layout(game);

    if (game->background)
        SDL_RenderCopy(renderer, game->background, NULL, &menuUi.backgroundRect);
    if (game->titleTexture)
        SDL_RenderCopy(renderer, game->titleTexture, NULL, &game->titleRect);
    if (game->logoTexture)
        SDL_RenderCopy(renderer, game->logoTexture, NULL, &game->logoRect);
    for (int i = 0; i < 5; i++) {
        SDL_Texture *t = game->buttons[i].selected
                         ? game->buttons[i].hoverTex
                         : game->buttons[i].normalTex;
        if (t) SDL_RenderCopy(renderer, t, NULL, &game->buttons[i].rect);
    }

    if (game->menuGiftTex) {
        SDL_Rect gift = menuUi.giftRect;
        if (menuUi.hoverGift) gift.y -= 4;
        SDL_RenderCopy(renderer, game->menuGiftTex, NULL, &gift);
    }
}
