#include "menu.h"
#include <stdio.h>

/* structure holding all assets used by the options menu */
typedef struct {
    SDL_Texture *background;
    SDL_Texture *plus;
    SDL_Texture *minus;
    SDL_Texture *mute;
    SDL_Texture *full;
    SDL_Texture *normal;
    Mix_Music   *music;
    Mix_Chunk   *click;
    TTF_Font    *font;
    SDL_Texture *title;
} MenuAssets;

static int mouse_over(SDL_Rect r, int x, int y)
{
    return x >= r.x && x <= r.x + r.w &&
           y >= r.y && y <= r.y + r.h;
}

/* helpers for loading / freeing assets */
static int menu_load_assets(SDL_Renderer *renderer, MenuAssets *a)
{
    a->background = IMG_LoadTexture(renderer, "options.png");
    a->plus       = IMG_LoadTexture(renderer, "volumeplus.png");
    a->minus      = IMG_LoadTexture(renderer, "volumeminus.png");
    a->mute       = IMG_LoadTexture(renderer, "volumemute.png");
    a->full       = IMG_LoadTexture(renderer, "fullscreen.png");
    a->normal     = IMG_LoadTexture(renderer, "normalscreen.png");

    if (!a->background || !a->plus || !a->minus || !a->mute ||
        !a->full || !a->normal) {
        fprintf(stderr, "Failed to load one or more textures\n");
        return -1;
    }

    a->music = Mix_LoadMUS("music.mp3");
    a->click = Mix_LoadWAV("sonbref.wav");
    if (!a->music || !a->click) {
        fprintf(stderr, "Failed to load audio\n");
        return -1;
    }

    a->font = TTF_OpenFont("arial.ttf", 60);
    if (!a->font) {
        fprintf(stderr, "TTF_OpenFont error: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Color white = {255, 255, 255};
    SDL_Surface *surf =
        TTF_RenderText_Blended(a->font, "OPTIONS", white);
    if (!surf) {
        fprintf(stderr, "Text render error: %s\n", TTF_GetError());
        return -1;
    }

    a->title = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    if (!a->title) {
        fprintf(stderr, "CreateTextureFromSurface failed: %s\n", SDL_GetError());
        return -1;
    }

    return 0;
}

static void menu_free_assets(MenuAssets *a)
{
    SDL_DestroyTexture(a->background);
    SDL_DestroyTexture(a->plus);
    SDL_DestroyTexture(a->minus);
    SDL_DestroyTexture(a->mute);
    SDL_DestroyTexture(a->full);
    SDL_DestroyTexture(a->normal);
    SDL_DestroyTexture(a->title);

    Mix_FreeMusic(a->music);
    Mix_FreeChunk(a->click);

    TTF_CloseFont(a->font);
}

int menu(SDL_Window *window, SDL_Renderer *renderer)
{
    SDL_Event e;
    int quit = 0;
    int fullscreen = 0;
    MenuAssets assets;

    if (menu_load_assets(renderer, &assets) != 0)
        return -1;

    Mix_PlayMusic(assets.music, -1);

    SDL_Rect titleRect = {0, 40, 400, 80};

    const int bw = 80, bh = 80;
    SDL_Rect btnPlus, btnMinus, btnMute, btnFull;

    while (!quit) {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        int centerX = w / 2;

        btnMinus = (SDL_Rect){centerX - 180, h / 2, bw, bh};
        btnMute  = (SDL_Rect){centerX - 40,  h / 2, bw, bh};
        btnPlus  = (SDL_Rect){centerX + 100, h / 2, bw, bh};
        btnFull  = (SDL_Rect){centerX - 40,  h / 2 + 120, bw, bh};

        titleRect.x = centerX - 200;

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                quit = 1;
            if (e.type == SDL_KEYDOWN &&
                e.key.keysym.sym == SDLK_ESCAPE)
                quit = 1;
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mx = e.button.x;
                int my = e.button.y;
                Mix_PlayChannel(-1, assets.click, 0);

                if (mouse_over(btnPlus, mx, my))
                    Mix_VolumeMusic(Mix_VolumeMusic(-1) + 10);
                if (mouse_over(btnMinus, mx, my))
                    Mix_VolumeMusic(Mix_VolumeMusic(-1) - 10);
                if (mouse_over(btnMute, mx, my))
                    Mix_VolumeMusic(0);
                if (mouse_over(btnFull, mx, my)) {
                    fullscreen = !fullscreen;
                    if (fullscreen)
                        SDL_SetWindowFullscreen(
                            window,
                            SDL_WINDOW_FULLSCREEN_DESKTOP);
                    else
                        SDL_SetWindowFullscreen(window, 0);
                }
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, assets.background, NULL, NULL);
        SDL_RenderCopy(renderer, assets.title, NULL, &titleRect);
        SDL_RenderCopy(renderer, assets.minus,  NULL, &btnMinus);
        SDL_RenderCopy(renderer, assets.mute,   NULL, &btnMute);
        SDL_RenderCopy(renderer, assets.plus,   NULL, &btnPlus);

        if (fullscreen)
            SDL_RenderCopy(renderer, assets.normal, NULL, &btnFull);
        else
            SDL_RenderCopy(renderer, assets.full,   NULL, &btnFull);

        SDL_RenderPresent(renderer);
    }

    menu_free_assets(&assets);
    return 0;
}
