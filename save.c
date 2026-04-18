#include "game.h"
#include <stdio.h>

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
