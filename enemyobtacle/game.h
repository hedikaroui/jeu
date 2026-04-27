#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// ================= STRUCTURES =================

typedef struct {
    int alive;
    int hurt;
    int neutralized;
} State;

typedef struct {
    SDL_Texture *textureNormal;
    SDL_Texture *textureHurt;
    SDL_Rect position;
    SDL_Rect posSprite;
    int frameWidth;
    int frameHeight;
    int frame;
    int nbCols;
    int nbRows;
    int direction;
    int health;
    State state;
} Enemy;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect position;
    int active;
    int colliding;
    int direction;
    int minX;
    int maxX;
    int speed;
    int baseY;
    double amplitude;
    double frequency;
    double phase;
} Obstacle;

typedef struct {
    int trees;
    SDL_Texture *treeTexture;
    SDL_Texture *backgroundTexture;
} HUD;

// ================= PROTOTYPES =================

// Enemy
void initEnemy(SDL_Renderer *renderer, Enemy *enemy, const char *normalPath, int normalCols, int normalRows, const char *hurtPath, int hurtCols, int hurtRows);
void moveAndAnimateEnemy(Enemy *enemy, HUD *hud);
void renderEnemy(SDL_Renderer *renderer, Enemy *enemy);
void checkEnemyHit(SDL_Renderer *renderer, Enemy *enemy, HUD *hud, const char *hurtPath);

// Obstacle
void initObstacle(SDL_Renderer *renderer, Obstacle *obs, const char *spritePath, int x, int y);
void renderObstacle(SDL_Renderer *renderer, Obstacle *obs);
void moveObstacle(Obstacle *obs);

// Collision
void checkCollision(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound);

// Background
void initBackground(HUD *hud, SDL_Renderer *renderer, const char *bgPath);
void renderBackground(SDL_Renderer *renderer, HUD *hud);

// HUD
void initHUD(HUD *hud, SDL_Renderer *renderer, const char *treePath);
void renderHUD(SDL_Renderer *renderer, HUD *hud);

// Cleanup
void destroyAll(Enemy *enemy, Obstacle *spider, Obstacle *falling, Mix_Chunk *collisionSound, HUD *hud);

#endif
