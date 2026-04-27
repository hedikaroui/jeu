#include "game.h"

void button_init(Button *btn, SDL_Renderer *renderer, int x, int y, int w, int h)
{
    (void)renderer;
    btn->texture = NULL;
    btn->rect.x = x;
    btn->rect.y = y;
    btn->rect.w = w;
    btn->rect.h = h;
    btn->selected = 0;
}

void button_render(SDL_Renderer *renderer, Button *btn)
{
    SDL_Color color = btn->selected ? (SDL_Color){255, 255, 0, 255} : (SDL_Color){200, 200, 200, 255};

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &btn->rect);

    if (btn->texture)
        SDL_RenderCopy(renderer, btn->texture, NULL, &btn->rect);
}

int button_clicked(Button *btn, int mx, int my)
{
    return (mx >= btn->rect.x && mx <= btn->rect.x + btn->rect.w &&
            my >= btn->rect.y && my <= btn->rect.y + btn->rect.h);
}
