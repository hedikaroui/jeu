#include "game.h"

void minimap_init(Minimap *mm, SDL_Renderer *renderer)
{
    (void)renderer;
    mm->rect.x = WIDTH - 150;
    mm->rect.y = 10;
    mm->rect.w = 140;
    mm->rect.h = 100;
    mm->x = 0;
    mm->y = 0;
}

void minimap_update(Minimap *mm, Player *p)
{
    mm->x = (p->position.x * mm->rect.w) / WIDTH + mm->rect.x;
    mm->y = (p->position.y * mm->rect.h) / HEIGHT + mm->rect.y;
}

void minimap_render(SDL_Renderer *renderer, Minimap *mm)
{
    SDL_Rect border = {mm->rect.x - 2, mm->rect.y - 2, mm->rect.w + 4, mm->rect.h + 4};
    SDL_Rect player_dot = {mm->x, mm->y, 6, 6};

    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderDrawRect(renderer, &border);
    SDL_RenderFillRect(renderer, &mm->rect);

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &player_dot);
}
