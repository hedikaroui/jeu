#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "header.h"

int pointInRect(int x, int y, SDL_Rect r)
{
    if (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h)
        return 1;
    return 0;
}

void renderText(SaveGame *s, char text[], int x, int y, SDL_Color color)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;

    surface = TTF_RenderText_Blended(s->font, text, color);
    if (!surface)
        return;

    texture = SDL_CreateTextureFromSurface(s->renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    rect.x = x;
    rect.y = y;
    rect.w = surface->w;
    rect.h = surface->h;

    SDL_RenderCopy(s->renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void renderTextCenter(SaveGame *s, char text[], SDL_Rect rect, SDL_Color color)
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect textRect;

    surface = TTF_RenderText_Blended(s->font, text, color);
    if (!surface)
        return;

    texture = SDL_CreateTextureFromSurface(s->renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    textRect.x = rect.x + (rect.w - surface->w) / 2;
    textRect.y = rect.y + (rect.h - surface->h) / 2;
    textRect.w = surface->w;
    textRect.h = surface->h;

    SDL_RenderCopy(s->renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

int InitSDL(SaveGame *s)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        printf("Erreur IMG_Init : %s\n", IMG_GetError());
        return 0;
    }
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        return 0;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        printf("Erreur Mix_OpenAudio : %s\n", Mix_GetError());
        return 0;
    }
    return 1;
}

int Initialisation(SaveGame *s)
{
    srand(time(NULL));

    s->background   = NULL;
    s->saveBackground = NULL;
    s->font         = NULL;
    s->titleSurface = NULL;
    s->titleTexture = NULL;
    s->music        = NULL;
    s->sound        = NULL;
    s->loadGameGrise = NULL;
    s->clic_bouton  = -1;
    s->selectedSlot = -1;

    for (int i = 0; i < 5; i++) {
        s->buttons[i].texture     = NULL;
        s->buttons[i].textureCliq = NULL;
        s->buttons[i].selected    = 0;
    }

    s->game_data.player_x = 100;
    s->game_data.player_y = 100;
    s->game_data.score = 0;
    s->game_data.lives = 3;

    for (int i = 0; i < 4; i++) {
        s->saves[i].id = i + 1;
        strcpy(s->saves[i].name, "Empty");
        s->saves[i].level = 0;
        s->saves[i].score = 0;
        strcpy(s->saves[i].date, "");
        s->saves[i].empty = 1;
    }

    s->running = 1;
    s->etat    = 0;

    return 1;
}

int Load(SaveGame *s)
{
    s->background = IMG_LoadTexture(s->renderer, "BG.png");
    if (!s->background) {
        printf("Erreur chargement BG.png : %s\n", IMG_GetError());
        return 0;
    }

    s->saveBackground = IMG_LoadTexture(s->renderer, "interface save sans boutons.png");
    if (!s->saveBackground) {
        printf("Erreur chargement interface save sans boutons.png : %s\n", IMG_GetError());
        return 0;
    }

    s->music = Mix_LoadMUS("music.mp3");
    if (!s->music) {
        printf("Erreur chargement music.mp3 : %s\n", Mix_GetError());
        return 0;
    }
    Mix_PlayMusic(s->music, -1);

    s->sound = Mix_LoadWAV("cliq.wav");
    if (!s->sound) {
        printf("Erreur chargement cliq.wav : %s\n", Mix_GetError());
        return 0;
    }

    s->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!s->font) {
        printf("Erreur chargement font : %s\n", TTF_GetError());
        return 0;
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red   = {178, 34, 34, 255};
    s->titleSurface = TTF_RenderText_Shaded(s->font, "Do you want to load a game", white, red);
    if (!s->titleSurface) {
        printf("Erreur titleSurface : %s\n", TTF_GetError());
        return 0;
    }
    s->titleTexture = SDL_CreateTextureFromSurface(s->renderer, s->titleSurface);
    s->titleRect    = (SDL_Rect){ (WIDTH - s->titleSurface->w) / 2, 20,
                                   s->titleSurface->w, s->titleSurface->h };

    s->buttons[0].rect        = (SDL_Rect){190, 390, 180, 72};
    s->buttons[0].texture     = IMG_LoadTexture(s->renderer, "yesno.png");
    s->buttons[0].textureCliq = IMG_LoadTexture(s->renderer, "yesyes.png");

    s->buttons[1].rect        = (SDL_Rect){430, 390, 180, 72};
    s->buttons[1].texture     = IMG_LoadTexture(s->renderer, "nono.png");
    s->buttons[1].textureCliq = IMG_LoadTexture(s->renderer, "noyes.png");

    s->buttons[2].rect        = (SDL_Rect){55, 530, 200, 50};
    s->buttons[2].texture     = IMG_LoadTexture(s->renderer, "loadgameno.png");
    s->buttons[2].textureCliq = IMG_LoadTexture(s->renderer, "loadgameyes.png");
    s->loadGameGrise          = IMG_LoadTexture(s->renderer, "loadgamegrise.png");

    s->buttons[3].rect        = (SDL_Rect){300, 530, 200, 50};
    s->buttons[3].texture     = IMG_LoadTexture(s->renderer, "Newgameno.png");
    s->buttons[3].textureCliq = IMG_LoadTexture(s->renderer, "Nouvpartieyes.png");

    s->buttons[4].rect        = (SDL_Rect){545, 530, 200, 50};
    s->buttons[4].texture     = IMG_LoadTexture(s->renderer, "quitno.png");
    s->buttons[4].textureCliq = IMG_LoadTexture(s->renderer, "Quitteryes.png");

    s->slotRects[0] = (SDL_Rect){285, 178, 245, 46};
    s->slotRects[1] = (SDL_Rect){285, 240, 245, 46};
    s->slotRects[2] = (SDL_Rect){285, 300, 245, 46};
    s->slotRects[3] = (SDL_Rect){285, 360, 245, 46};

    if (!s->loadGameGrise) {
        printf("Erreur chargement loadgamegrise.png : %s\n", IMG_GetError());
        return 0;
    }

    for (int i = 0; i < 5; i++) {
        if (!s->buttons[i].texture || !s->buttons[i].textureCliq) {
            printf("Erreur chargement texture bouton %d : %s\n", i, IMG_GetError());
            return 0;
        }
    }

    loadSavesFromFile(s);

    return 1;
}

void Affichage(SaveGame *s)
{
    SDL_SetRenderDrawColor(s->renderer, 255, 255, 255, 255);
    SDL_RenderClear(s->renderer);
    if (s->etat == 1)
        SDL_RenderCopy(s->renderer, s->saveBackground, NULL, NULL);
    else
        SDL_RenderCopy(s->renderer, s->background, NULL, NULL);

    if (s->etat == 0) {
        SDL_RenderCopy(s->renderer, s->titleTexture, NULL, &s->titleRect);

        if (s->buttons[0].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[0].textureCliq, NULL, &s->buttons[0].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[0].texture, NULL, &s->buttons[0].rect);

        if (s->buttons[1].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[1].textureCliq, NULL, &s->buttons[1].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[1].texture, NULL, &s->buttons[1].rect);

    } else if (s->etat == 1) {
        displaySaveSlots(s);

        if (s->selectedSlot == -1) {
            SDL_RenderCopy(s->renderer, s->loadGameGrise, NULL, &s->buttons[2].rect);
        } else if (s->buttons[2].selected == 1) {
            SDL_RenderCopy(s->renderer, s->buttons[2].textureCliq, NULL, &s->buttons[2].rect);
        } else {
            SDL_RenderCopy(s->renderer, s->buttons[2].texture, NULL, &s->buttons[2].rect);
        }

        if (s->buttons[3].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[3].textureCliq, NULL, &s->buttons[3].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[3].texture, NULL, &s->buttons[3].rect);

        if (s->buttons[4].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[4].textureCliq, NULL, &s->buttons[4].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[4].texture, NULL, &s->buttons[4].rect);
    } else {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color green = {40, 180, 90, 255};
        char text[150];

        sprintf(text, "Game started : %s", s->saves[s->selectedSlot].name);
        renderText(s, text, 230, 230, white);

        sprintf(text, "Level %d   Score %d", s->saves[s->selectedSlot].level, s->game_data.score);
        renderText(s, text, 265, 280, green);
    }

    SDL_RenderPresent(s->renderer);
}

void LectureEntree(SaveGame *s)
{
    SDL_Event event;
    s->clic_bouton = -1;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT)
            s->running = 0;

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                s->running = 0;

            if (s->etat == 1) {
                selectSaveSlot(s, event);
                if (event.key.keysym.sym == SDLK_RETURN && s->selectedSlot != -1)
                    s->clic_bouton = 2;
                if (event.key.keysym.sym == SDLK_n)
                    s->clic_bouton = 3;
                if (event.key.keysym.sym == SDLK_e)
                    s->clic_bouton = 4;
            }
        }

        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x, my = event.motion.y;
            for (int i = 0; i < 5; i++) s->buttons[i].selected = 0;

            if (s->etat == 0) {
                if (pointInRect(mx, my, s->buttons[0].rect))
                    s->buttons[0].selected = 1;

                if (pointInRect(mx, my, s->buttons[1].rect))
                    s->buttons[1].selected = 1;
            } else if (s->etat == 1) {
                if (pointInRect(mx, my, s->buttons[2].rect))
                    s->buttons[2].selected = 1;

                if (pointInRect(mx, my, s->buttons[3].rect))
                    s->buttons[3].selected = 1;

                if (pointInRect(mx, my, s->buttons[4].rect))
                    s->buttons[4].selected = 1;
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;

            if (s->etat == 0) {
                if (pointInRect(mx, my, s->buttons[0].rect))
                    s->clic_bouton = 0;

                if (pointInRect(mx, my, s->buttons[1].rect))
                    s->clic_bouton = 1;
            } else if (s->etat == 1) {
                for (int i = 0; i < 4; i++) {
                    if (pointInRect(mx, my, s->slotRects[i]))
                        s->selectedSlot = i;
                }

                if (pointInRect(mx, my, s->buttons[2].rect) && s->selectedSlot != -1)
                    s->clic_bouton = 2;

                if (pointInRect(mx, my, s->buttons[3].rect))
                    s->clic_bouton = 3;

                if (pointInRect(mx, my, s->buttons[4].rect))
                    s->clic_bouton = 4;
            }
        }
    }
}

void MiseAJour(SaveGame *s)
{
    if (s->clic_bouton == 0) {
        Mix_PlayChannel(-1, s->sound, 0);
        loadSavesFromFile(s);
        s->selectedSlot = -1;
        s->etat = 1;
    }
    if (s->clic_bouton == 1) {
        Mix_PlayChannel(-1, s->sound, 0);
        printf("You chose no \n");
    }
    if (s->clic_bouton == 2) {
        Mix_PlayChannel(-1, s->sound, 0);
        if (s->selectedSlot != -1 && s->saves[s->selectedSlot].empty == 0) {
            loadGameById(s, s->saves[s->selectedSlot].id);
            s->etat = 2;
        } else {
            printf("Empty Slot\n");
        }
    }
    if (s->clic_bouton == 3) {
        Mix_PlayChannel(-1, s->sound, 0);
        if (createNewSave(s)) {
            loadSavesFromFile(s);
            loadGameById(s, s->saves[s->selectedSlot].id);
            s->etat = 2;
        }
    }
    if (s->clic_bouton == 4) {
        Mix_PlayChannel(-1, s->sound, 0);
        s->running = 0;
    }

    SDL_Delay(16);
}

void Liberation(SaveGame *s)
{
    SDL_FreeSurface(s->titleSurface);
    SDL_DestroyTexture(s->titleTexture);
    SDL_DestroyTexture(s->background);
    SDL_DestroyTexture(s->saveBackground);
    SDL_DestroyTexture(s->loadGameGrise);

    for (int i = 0; i < 5; i++) {
        SDL_DestroyTexture(s->buttons[i].texture);
        SDL_DestroyTexture(s->buttons[i].textureCliq);
    }

    TTF_CloseFont(s->font);
    Mix_FreeMusic(s->music);
    Mix_FreeChunk(s->sound);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(s->renderer);
    SDL_DestroyWindow(s->window);
    SDL_Quit();
}

int save_game(GameData *data)
{
    FILE *file = fopen("savegame.txt", "w");
    if (!file) {
        printf("Erreur: impossible d'ouvrir le fichier de sauvegarde\n");
        return 0;
    }

    fprintf(file, "=== SAUVEGARDE DU JEU ===\n");
    fprintf(file, "Position du joueur: X=%d, Y=%d\n", data->player_x, data->player_y);
    fprintf(file, "Score: %d\n", data->score);
    fprintf(file, "Vies: %d\n", data->lives);
    fprintf(file, "========================\n");

    fclose(file);
    return 1;
}

int load_game(GameData *data)
{
    FILE *file = fopen("savegame.txt", "r");
    if (!file) {
        printf("Erreur: fichier de sauvegarde introuvable\n");
        return 0;
    }

    if (fscanf(file, "=== SAUVEGARDE DU JEU ===\n") == 0) {
        printf("Erreur: format de fichier invalide\n");
        fclose(file);
        return 0;
    }

    if (fscanf(file, "Position du joueur: X=%d, Y=%d\n", &data->player_x, &data->player_y) != 2) {
        printf("Erreur: impossible de lire la position\n");
        fclose(file);
        return 0;
    }

    if (fscanf(file, "Score: %d\n", &data->score) != 1) {
        printf("Erreur: impossible de lire le score\n");
        fclose(file);
        return 0;
    }

    if (fscanf(file, "Vies: %d\n", &data->lives) != 1) {
        printf("Erreur: impossible de lire les vies\n");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

int loadSavesFromFile(SaveGame *s)
{
    FILE *file;
    char line[150];
    char name[50];
    char date[30];
    int id;
    int level;
    int score;
    int count;
    int i;
    int slot;
    int lineNumber;

    for (i = 0; i < 4; i++) {
        s->saves[i].id = 0;
        strcpy(s->saves[i].name, "Empty");
        s->saves[i].level = 0;
        s->saves[i].score = 0;
        strcpy(s->saves[i].date, "");
        s->saves[i].empty = 1;
    }

    file = fopen("savegame.txt", "r");
    if (!file) {
        printf("Fichier savegame.txt introuvable, slots vides\n");
        return 0;
    }

    lineNumber = 0;
    while (fgets(line, 150, file)) {
        strcpy(date, "");
        count = sscanf(line, "%d %49s %d %d %29s", &id, name, &level, &score, date);

        if (count >= 4) {
            slot = lineNumber % 4;
            s->saves[slot].id = id;
            strcpy(s->saves[slot].name, name);
            s->saves[slot].level = level;
            s->saves[slot].score = score;
            strcpy(s->saves[slot].date, date);
            s->saves[slot].empty = 0;
            lineNumber++;
        }
    }

    fclose(file);
    return 1;
}

void displaySaveSlots(SaveGame *s)
{
    int i;
    SDL_Color black = {20, 20, 20, 255};
    char text[150];
    SDL_Rect border;

    for (i = 0; i < 4; i++) {
        if (i == s->selectedSlot) {
            SDL_SetRenderDrawColor(s->renderer, 185, 130, 25, 255);
            border = s->slotRects[i];
            SDL_RenderDrawRect(s->renderer, &border);
            border.x = border.x + 1;
            border.y = border.y + 1;
            border.w = border.w - 2;
            border.h = border.h - 2;
            SDL_RenderDrawRect(s->renderer, &border);
        }

        if (s->saves[i].empty == 1) {
            sprintf(text, "Empty");
        } else {
            sprintf(text, "%d   %s", s->saves[i].id, s->saves[i].name);
        }

        renderTextCenter(s, text, s->slotRects[i], black);
    }
}

void selectSaveSlot(SaveGame *s, SDL_Event event)
{
    if (event.key.keysym.sym == SDLK_DOWN) {
        s->selectedSlot++;
        if (s->selectedSlot > 3)
            s->selectedSlot = 0;
    }

    if (event.key.keysym.sym == SDLK_UP) {
        s->selectedSlot--;
        if (s->selectedSlot < 0)
            s->selectedSlot = 3;
    }

    if (event.key.keysym.sym == SDLK_1)
        s->selectedSlot = 0;
    if (event.key.keysym.sym == SDLK_2)
        s->selectedSlot = 1;
    if (event.key.keysym.sym == SDLK_3)
        s->selectedSlot = 2;
    if (event.key.keysym.sym == SDLK_4)
        s->selectedSlot = 3;
}

int loadGameById(SaveGame *s, int id)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (s->saves[i].id == id && s->saves[i].empty == 0) {
            s->game_data.player_x = 100 + s->saves[i].level * 20;
            s->game_data.player_y = 100;
            s->game_data.score = s->saves[i].score;
            s->game_data.lives = 3;
            s->selectedSlot = i;

            printf("Chargement sauvegarde ID %d : %s niveau %d score %d\n",
                   s->saves[i].id,
                   s->saves[i].name,
                   s->saves[i].level,
                   s->saves[i].score);
            return 1;
        }
    }

    printf("Sauvegarde ID %d introuvable\n", id);
    return 0;
}

int generateUniqueId(SaveGame *s)
{
    FILE *file;
    char line[150];
    char name[50];
    int i;
    int j;
    int id;
    int level;
    int score;
    int newId;
    int used;

    for (i = 0; i < 100; i++) {
        newId = 100000 + rand() % 900000;
        used = 0;

        file = fopen("savegame.txt", "r");
        if (file) {
            while (fgets(line, 150, file)) {
                if (sscanf(line, "%d %49s %d %d", &id, name, &level, &score) == 4) {
                    if (id == newId)
                        used = 1;
                }
            }
            fclose(file);
        }

        for (j = 0; j < 4; j++) {
            if (s->saves[j].empty == 0 && s->saves[j].id == newId)
                used = 1;
        }

        if (used == 0)
            return newId;
    }

    for (newId = 100000; newId <= 999999; newId++) {
        used = 0;

        file = fopen("savegame.txt", "r");
        if (file) {
            while (fgets(line, 150, file)) {
                if (sscanf(line, "%d %49s %d %d", &id, name, &level, &score) == 4) {
                    if (id == newId)
                        used = 1;
                }
            }
            fclose(file);
        }

        if (used == 0)
            return newId;
    }

    return -1;
}

int countSavesInFile()
{
    FILE *file;
    char line[150];
    char name[50];
    int id;
    int level;
    int score;
    int count;

    count = 0;
    file = fopen("savegame.txt", "r");
    if (!file)
        return 0;

    while (fgets(line, 150, file)) {
        if (sscanf(line, "%d %49s %d %d", &id, name, &level, &score) == 4)
            count++;
    }

    fclose(file);
    return count;
}

int createNewSave(SaveGame *s)
{
    FILE *file;
    int slot;
    int newId;
    char name[50];

    newId = generateUniqueId(s);
    if (newId < 0) {
        printf("Impossible de creer un ID unique\n");
        return 0;
    }

    slot = countSavesInFile() % 4;
    strcpy(name, "Player");

    s->saves[slot].id = newId;
    strcpy(s->saves[slot].name, name);
    s->saves[slot].level = 1;
    s->saves[slot].score = 0;
    strcpy(s->saves[slot].date, "");
    s->saves[slot].empty = 0;
    s->selectedSlot = slot;

    file = fopen("savegame.txt", "a");
    if (!file) {
        printf("Erreur creation nouvelle sauvegarde\n");
        return 0;
    }

    fprintf(file, "%d %s %d %d\n",
            s->saves[slot].id,
            s->saves[slot].name,
            s->saves[slot].level,
            s->saves[slot].score);

    fclose(file);

    s->game_data.player_x = 100;
    s->game_data.player_y = 100;
    s->game_data.score = 0;
    s->game_data.lives = 3;

    printf("Nouvelle sauvegarde creee : ID %d\n", s->saves[slot].id);
    return 1;
}
