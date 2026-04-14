#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#define MAX_SNOWBALLS 20



typedef struct {
    int alive;
    int hurt;      
    int neutralized;
    int playerTouch; 
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
} Player;




void initEnemy(SDL_Renderer *renderer, Enemy *enemy,const char *normalPath, int normalCols, int normalRows,const char *hurtPath, int hurtCols, int hurtRows);
void moveAndAnimateEnemy(Enemy *enemy);
void renderEnemy(SDL_Renderer *renderer, Enemy *enemy);

void initObstacle(SDL_Renderer *renderer, Obstacle *obs, const char *spritePath, int x, int y);
void renderObstacle(SDL_Renderer *renderer, Obstacle *obs);
void moveObstacle(Obstacle *obs);


void checkCollision(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound);


void scrollBackground(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs);


void initHUD(HUD *hud, SDL_Renderer *renderer, const char *treePath);
void renderHUD(SDL_Renderer *renderer, HUD *hud);


void initSnowball(SDL_Renderer *renderer, Snowball *sb, const char *spritePath);
void handleSnowballThrow(SDL_Event *event, Player *player, Snowball snowballs[], int *snowballCount);
void updateSnowballs(Snowball snowballs[], int *snowballCount);
void renderSnowballs(SDL_Renderer *renderer, Snowball snowballs[], int snowballCount);
void checkSnowballEnemyCollision(Snowball snowballs[], int *snowballCount, Enemy *enemy, HUD *hud);


void initPlayer(SDL_Renderer *renderer, Player *player, const char *spritePath, int nbCols, int nbRows, Enemy *enemy);
void handlePlayerMovement(SDL_Event *event, Player *player);  
void updatePlayer(Player *player, Enemy *enemy);               
void renderPlayer(SDL_Renderer *renderer, Player *player);    
void checkPlayerEnemyCollision(Player *player, Enemy *enemy);  


void destroyAll(Enemy *enemy, Obstacle *spider, Obstacle *falling, Mix_Chunk *collisionSound, HUD *hud, Snowball *snowball);

#endif
