#ifndef ENEMI_H
#define ENEMI_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define MAX_SNOWBALLS 20

// ================= STRUCTURES =================

typedef struct {
    int alive;
    int hurt;        // blessé par boule de neige
    int neutralized;
    int playerTouch; // 🔥 nouveau : touché par joueur
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
} Obstacle;

typedef struct {
    int trees;
    SDL_Texture *treeTexture;
} HUD;

typedef struct {
    int x, y;
    int speed;
    int active;
    SDL_Rect rect;
    SDL_Texture *texture;
} Snowball;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect position;
    int frame;
    int maxFrames;
    int frameWidth;
    int frameHeight;
    int speed;
    int isRunning;
    int direction;
} EPlayer;

// ================= PROTOTYPES =================

// Enemy
void initEnemy(SDL_Renderer *renderer, Enemy *enemy,const char *normalPath, int normalCols, int normalRows,const char *hurtPath, int hurtCols, int hurtRows);
void moveAndAnimateEnemy(Enemy *enemy);
void renderEnemy(SDL_Renderer *renderer, Enemy *enemy, SDL_Rect *cam);
// Obstacle
void initObstacle(SDL_Renderer *renderer, Obstacle *obs, const char *spritePath, int x, int y);
void renderObstacle(SDL_Renderer *renderer, Obstacle *obs);
void moveObstacle(Obstacle *obs);

// Collision
void checkCollision(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound);

// Background
void scrollBackground(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs);

// HUD
void initHUD(HUD *hud, SDL_Renderer *renderer, const char *treePath);
void renderHUD(SDL_Renderer *renderer, HUD *hud);

// Snowball
void initSnowball(SDL_Renderer *renderer, Snowball *sb, const char *spritePath);
void handleSnowballThrow(SDL_Event *event, EPlayer *player, Snowball snowballs[], int *snowballCount);
void updateSnowballs(Snowball snowballs[], int *snowballCount);
void renderSnowballs(SDL_Renderer *renderer, Snowball snowballs[], int snowballCount);
void checkSnowballEnemyCollision(Snowball snowballs[], int *snowballCount, Enemy *enemy, HUD *hud);

// Player
void initPlayer(SDL_Renderer *renderer, EPlayer *player, const char *spritePath, int nbCols, int nbRows, Enemy *enemy);
void handlePlayerMovement(SDL_Event *event, EPlayer *player);   // gère les flèches
void updatePlayer(EPlayer *player, Enemy *enemy);               // avance derrière l’ennemi
void renderPlayer(SDL_Renderer *renderer, EPlayer *player);     // affiche le joueur
void checkPlayerEnemyCollision(EPlayer *player, Enemy *enemy);  // collision avec l’ennemi

// Cleanup
void destroyAll(Enemy *enemy, Obstacle *spider, Obstacle *falling, Mix_Chunk *collisionSound, HUD *hud, Snowball *snowball);

#endif


