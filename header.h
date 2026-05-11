#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define WIDTH  800
#define HEIGHT 600

typedef struct {
    SDL_Rect     rect;
    int          selected;
    SDL_Texture *texture;
    SDL_Texture *textureCliq;
} Button;

typedef struct {
    int player_x;
    int player_y;
    int score;
    int lives;
} GameData;

typedef struct {
    int id;
    char name[50];
    int level;
    int score;
    char date[30];
    int empty;
} SaveSlot;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;

    SDL_Texture  *background;
    SDL_Texture  *saveBackground;
    TTF_Font     *font;

    SDL_Surface  *titleSurface;
    SDL_Texture  *titleTexture;
    SDL_Rect      titleRect;

    Mix_Music    *music;
    Mix_Chunk    *sound;

    Button buttons[5];
    SDL_Texture *loadGameGrise;
    SaveSlot saves[4];
    SDL_Rect slotRects[4];

    int running;
    int etat;
    int clic_bouton;
    int selectedSlot;

    GameData game_data;

} SaveGame;

int  InitSDL        (SaveGame *s);
int  Initialisation (SaveGame *s);
int  Load           (SaveGame *s);
void Affichage      (SaveGame *s);
void LectureEntree  (SaveGame *s);
void MiseAJour      (SaveGame *s);
void Liberation     (SaveGame *s);
int  save_game      (GameData *data);
int  load_game      (GameData *data);
int  loadSavesFromFile(SaveGame *s);
void displaySaveSlots(SaveGame *s);
void selectSaveSlot(SaveGame *s, SDL_Event event);
int  loadGameById(SaveGame *s, int id);
int  generateUniqueId(SaveGame *s);
int  countSavesInFile();
int  createNewSave(SaveGame *s);

#endif
