#include "header.h"
#include <stdio.h>

/* helpers for loading textures, music and sound effects */
SDL_Texture* ChargerTexture(SDL_Renderer *renderer, const char *fichier)
{
    SDL_Texture *tex = IMG_LoadTexture(renderer, fichier);
    if (!tex)
        fprintf(stderr, "Erreur chargement %s : %s\n", fichier, IMG_GetError());
    return tex;
}

Mix_Music* ChargerMusique(const char *fichier)
{
    Mix_Music *mus = Mix_LoadMUS(fichier);
    if (!mus)
        fprintf(stderr, "Erreur musique %s : %s\n", fichier, Mix_GetError());
    return mus;
}

Mix_Chunk* ChargerSon(const char *fichier)
{
    Mix_Chunk *son = Mix_LoadWAV(fichier);
    if (!son)
        fprintf(stderr, "Erreur son %s : %s\n", fichier, Mix_GetError());
    return son;
}
