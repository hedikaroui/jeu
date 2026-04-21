#include "game.h"

void quiz_init(Quiz *q, char *question, char *opt1, char *opt2, char *opt3, char *opt4, int correct)
{
    strncpy(q->question, question, 255);
    strncpy(q->options[0], opt1, 255);
    strncpy(q->options[1], opt2, 255);
    strncpy(q->options[2], opt3, 255);
    strncpy(q->options[3], opt4, 255);
    q->correct_answer = correct;
    q->answered = 0;
}

void quiz_answer(Quiz *q, int choice)
{
    if (choice >= 0 && choice < 4)
        q->answered = 1;
}

void quiz_render(SDL_Renderer *renderer, Quiz *q, TTF_Font *font)
{
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect = {100, 100, 0, 0};
    int i;

    if (!font) return;

    surface = TTF_RenderText_Solid(font, q->question, white);
    if (surface)
    {
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        rect.w = surface->w;
        rect.h = surface->h;
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    for (i = 0; i < 4; i++)
    {
        surface = TTF_RenderText_Solid(font, q->options[i], white);
        if (surface)
        {
            texture = SDL_CreateTextureFromSurface(renderer, surface);
            rect.y += 50;
            rect.w = surface->w;
            rect.h = surface->h;
            SDL_RenderCopy(renderer, texture, NULL, &rect);
            SDL_DestroyTexture(texture);
            SDL_FreeSurface(surface);
        }
    }
}
