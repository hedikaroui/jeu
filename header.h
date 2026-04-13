#ifndef HEADER_H
#define HEADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define WIDTH  800
#define HEIGHT 600
typedef struct {
    char question[300];
    char repA[150];
    char repB[150];
    char repC[150];
    int  bonneReponse;
} Question;

typedef struct {

    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *police;
    SDL_Texture *bg1;
    SDL_Texture *bg2;
    SDL_Texture *btnQuiz;
    SDL_Texture *btnQuizHover;
    SDL_Texture *btnPuzzle;
    SDL_Texture *btnPuzzleHover;
    SDL_Texture *btnA;
    SDL_Texture *btnB;
    SDL_Texture *btnC;
    SDL_Texture *loser;
    Mix_Chunk *beep;
    Mix_Chunk *beep2;
    Mix_Chunk *laugh;
    Mix_Music *musique;
    Question questions[50];
    int      nbQuestions;
    int      dejaPosee[50];
    Question actuelle;
    int etat;   
    int running;
    int score;
    int feedback;       
    int feedbackTimer;
    Uint32 tempsDepart;
    int    tempsRestant;
    int    enigmeFinie;  
    int repSurvol; 
    SDL_Rect rectQuiz;
    SDL_Rect rectPuzzle;
    SDL_Rect rectA;
    SDL_Rect rectB;
    SDL_Rect rectC;

} enigme;
int  InitSDL         (enigme *e);
int  Initialisation  (enigme *e);
int  Load            (enigme *e);
void Affichage       (enigme *e);
void GererEvenement  (enigme *e, int *running);
void MiseAJour       (enigme *e);
void Liberation      (enigme *e);
void NouvelleQuestion(enigme *e);
void AfficherTexte   (enigme *e, const char *txt, int x, int y, SDL_Color col, int taille);
void AfficherTimeline(enigme *e);

#endif
