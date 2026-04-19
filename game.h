#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define MAX_SNOWBALLS 20

typedef struct {
    SDL_Texture *textureNormal;
    SDL_Texture *textureHurt;
    SDL_Texture *treeTexture;
    SDL_Rect     position;
    SDL_Rect     posSprite;
    int frameWidth;
    int frameHeight;
    int frame;
    int nbCols;
    int nbRows;
    int direction;
    int health;
    int alive;
    int hurt;
    int neutralized;
    int playerTouch;
    int hurtTimer;
    int playerTouchTimer;
    int recoilTimer;
    int recoiling;
    int blinkTimer;
    int blinking;
    int trees;
    int currentSpeed;
} Enemy;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect     position;
    int active;
} Obstacle;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect     rect;
    int x, y;
    int speed;
    int active;
} Snowball;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect     position;
    int frame;
    int maxFrames;
    int frameWidth;
    int frameHeight;
    int speed;
    int isRunning;
    int direction;
    int acceleration;
    int maxSpeed;
} Player;

void init(SDL_Renderer *renderer,
          Enemy    *enemy,  char *normalPath, int normalCols, int normalRows,
                            char *hurtPath,   int hurtCols,   int hurtRows,
                            char *treePath,
          Obstacle *obs,    char *obsPath,    int x,          int y,
          Snowball *sb,     char *snowPath,
          Player   *player, char *playerPath, int nbCols,     int nbRows);

void affichage(SDL_Renderer *renderer,
               Enemy *enemy, Obstacle *obs,
               Snowball snowballs[], int snowballCount,
               Player *player);

void move(Enemy *enemy, Obstacle *obs);

void updatePlayer(Player *player, Enemy *enemy,
                  Snowball snowballs[], int *snowballCount);

void checkCollision(SDL_Renderer *renderer,
                    Enemy *enemy, Obstacle *obs,
                    Mix_Chunk *collisionSound,
                    Snowball snowballs[], int *snowballCount,
                    Player *player);

void handleInput(SDL_Event *event, Player *player,
                 Snowball snowballs[], int *snowballCount);

void scrollBackground(SDL_Renderer *renderer, Enemy *enemy,
                      Obstacle *obs);

void destroyAll(Enemy *enemy, Obstacle *obs,
                Mix_Chunk *collisionSound, Snowball *snowball);

void initObstacle(SDL_Renderer *renderer, Obstacle *obs, char *path, int x, int y);

#endif

