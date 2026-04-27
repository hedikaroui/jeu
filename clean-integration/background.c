#include "game.h"

void background_init(Background *bg, SDL_Renderer *renderer, char *path)
{
    bg->texture = IMG_LoadTexture(renderer, path);
    bg->scroll_x = 0;
    bg->scroll_y = 0;
}

void background_update(Background *bg)
{
    bg->scroll_x += 1;
    if (bg->scroll_x > WIDTH)
        bg->scroll_x = 0;
}

void background_render(SDL_Renderer *renderer, Background *bg)
{
    SDL_Rect dest = {bg->scroll_x, bg->scroll_y, WIDTH, HEIGHT};

    if (bg->texture)
        SDL_RenderCopy(renderer, bg->texture, NULL, &dest);

    if (bg->scroll_x > 0)
    {
        SDL_Rect dest2 = {bg->scroll_x - WIDTH, bg->scroll_y, WIDTH, HEIGHT};
        SDL_RenderCopy(renderer, bg->texture, NULL, &dest2);
    }
}
