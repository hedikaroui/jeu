#include "game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>

// 
void initEnemy(SDL_Renderer *renderer,Enemy *enemy,const char *normalPath,int normalCols,int normalRows,const char *hurtPath,int hurtCols,int hurtRows){
    SDL_Surface *surface = IMG_Load(normalPath);
    enemy->textureNormal = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_QueryTexture(enemy->textureNormal, NULL, NULL, &enemy->frameWidth, &enemy->frameHeight);
    enemy->frameWidth  /= normalCols;
    enemy->frameHeight /= normalRows;

    surface = IMG_Load(hurtPath);
    enemy->textureHurt = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    enemy->position.x = 300;
    enemy->position.y = 260;
    enemy->position.w = enemy->frameWidth;
    enemy->position.h = enemy->frameHeight;

    enemy->posSprite = (SDL_Rect){0, 0, enemy->frameWidth, enemy->frameHeight};
    enemy->frame = 0;
    enemy->nbCols = normalCols;
    enemy->nbRows = normalRows;
    enemy->direction = 0;

    enemy->health = 100;
    enemy->state.alive = 1;
    enemy->state.hurt = 0;
    enemy->state.neutralized = 0;
    enemy->state.playerTouch = 0;
}
//
void renderEnemy(SDL_Renderer *renderer, Enemy *enemy) {
    if (enemy->state.alive) {
        if (enemy->state.hurt) {
           
            SDL_SetTextureColorMod(enemy->textureHurt, 255, 0, 0);
            SDL_SetTextureAlphaMod(enemy->textureHurt, 180);
            SDL_RenderCopy(renderer, enemy->textureHurt, &enemy->posSprite, &enemy->position);
        } else if (enemy->state.playerTouch) {
          
            SDL_SetTextureColorMod(enemy->textureNormal, 255, 0, 0);
            SDL_SetTextureAlphaMod(enemy->textureNormal, 180);
            SDL_RenderCopy(renderer, enemy->textureNormal, &enemy->posSprite, &enemy->position);
        } else {
       
            SDL_SetTextureColorMod(enemy->textureNormal, 255, 255, 255);
            SDL_SetTextureAlphaMod(enemy->textureNormal, 255);
            SDL_RenderCopy(renderer, enemy->textureNormal, &enemy->posSprite, &enemy->position);
        }
    }
}
//
void moveAndAnimateEnemy(Enemy *enemy) {
    static int recoiling = 0;
    static int recoilTimer = 0;
    static int hurtTimer = 0;
    static int playerTouchTimer = 0;

    if (enemy->health > 4) {
      
        enemy->direction = 0; 
        enemy->position.x += 1;
    } else {
      
        if (!recoiling && recoilTimer == 0) {
            recoiling = 1;
            recoilTimer = 40;
        }

        if (recoiling) {
           
            enemy->direction = 1;
            enemy->position.x -= 8;
            recoilTimer--;
            if (recoilTimer <= 0) {
                recoiling = 0; 
            }
        } else {
          
            enemy->direction = 0;
            enemy->position.x += 15;
        }
    }

   
    if (enemy->position.x > 800) {
        enemy->position.x = -enemy->position.w;
    }

 
    enemy->frame++;
    if (enemy->frame >= enemy->nbCols) enemy->frame = 0;
    enemy->posSprite.x = enemy->frame * enemy->frameWidth;


    enemy->posSprite.y = enemy->direction * enemy->frameHeight;


    if (enemy->state.hurt) {
        hurtTimer++;
        if (hurtTimer > 70) {
            enemy->state.hurt = 0;
            hurtTimer = 0;
        }
    }

   
    if (enemy->state.playerTouch) {
        playerTouchTimer++;
        if (playerTouchTimer > 15) {
            enemy->state.playerTouch = 0;
            playerTouchTimer = 0;
        }
    }
}
//
void initObstacle(SDL_Renderer *renderer, Obstacle *obs, const char *spritePath, int x, int y) {
    SDL_Surface *surface = IMG_Load(spritePath);
    if (!surface) return;
    obs->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    obs->position.x = x;
    obs->position.y = y;
    obs->position.w = 128;
    obs->position.h = 128;
    obs->active = 1;
}

// 
void renderObstacle(SDL_Renderer *renderer, Obstacle *obs) {
    if (obs->active) {
        SDL_RenderCopy(renderer, obs->texture, NULL, &obs->position);
    }
}

// 
void moveObstacle(Obstacle *obs) {
    if (!obs->active) return;
    obs->position.x -= 1;
    if (obs->position.x < -128) obs->position.x = 800;
    obs->position.y = 380;
}

//
void checkCollision(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound) {
    if (!obs->active) return;
    if (SDL_HasIntersection(&enemy->position, &obs->position)) {
        if (collisionSound) {
            Mix_PlayChannel(-1, collisionSound, 0);
        }
    }
}

void scrollBackground(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs) {
    static SDL_Texture *background = NULL;
    static int currentBg = 1;

    if (!background) {
        SDL_Surface *surface = IMG_Load("background1.png");
        if (surface) {
            background = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }

    if (enemy->position.x >= 800) {
        currentBg++;
        if (currentBg > 4) currentBg = 1; 

        char filename[30];
        sprintf(filename, "background%d.png", currentBg);

        SDL_Surface *surface = IMG_Load(filename);
        if (surface) {
            SDL_DestroyTexture(background);
            background = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }

        enemy->position.x = 0;

      
        switch (currentBg) {
            case 1: initObstacle(renderer, obs, "falling.png", 600, 400); break;
            case 2: initObstacle(renderer, obs, "spider.png", 600, 400); break;
            case 3: initObstacle(renderer, obs, "falling.png", 600, 400); break;
            case 4: initObstacle(renderer, obs, "spider.png", 600, 400); break; 
        }
    }

    if (background) {
        SDL_Rect bgRect = {0, 0, 800, 600};
        SDL_RenderCopy(renderer, background, NULL, &bgRect);
    }
}
//

void initHUD(HUD *hud, SDL_Renderer *renderer, const char *treePath) {
    hud->trees = 7;
    SDL_Surface *surface = IMG_Load(treePath);
    if (!surface) return;
    hud->treeTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

void renderHUD(SDL_Renderer *renderer, HUD *hud) {
    for (int i = 0; i < hud->trees; i++) {
        SDL_Rect dst = {10 + i*80, 10, 80, 80};
        SDL_RenderCopy(renderer, hud->treeTexture, NULL, &dst);
    }
}


void initSnowball(SDL_Renderer *renderer, Snowball *sb, const char *spritePath) {
    SDL_Surface *surface = IMG_Load(spritePath);
    if (!surface) return;
    sb->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

  
    sb->rect.w = 32; 
    sb->rect.h = 32;   

    sb->x = sb->y = 0;
    sb->speed = 0;
    sb->active = 0;
    sb->rect.x = 0;
    sb->rect.y = 0;
}


void handleSnowballThrow(SDL_Event *event, Player *player, Snowball snowballs[], int *snowballCount) {
    if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_s) {
        if (*snowballCount < MAX_SNOWBALLS) {
            Snowball *sb = &snowballs[*snowballCount];
            sb->x = player->position.x + player->position.w; 
            sb->y = player->position.y + (player->position.h / 2) - (sb->rect.h / 2); 
            sb->speed = 8;  
            sb->active = 1;
            sb->rect.x = sb->x;
            sb->rect.y = sb->y;
            sb->texture = snowballs[0].texture; 
            (*snowballCount)++;
        }
    }
}


void updateSnowballs(Snowball snowballs[], int *snowballCount) {
    for (int i = 0; i < *snowballCount; i++) {
        if (snowballs[i].active) {
            snowballs[i].x += snowballs[i].speed;
            snowballs[i].rect.x = snowballs[i].x;

         
            if (snowballs[i].x > 800) {
                snowballs[i].active = 0;
            }
        }
    }
}


void renderSnowballs(SDL_Renderer *renderer, Snowball snowballs[], int snowballCount) {
    for (int i = 0; i < snowballCount; i++) {
        if (snowballs[i].active && snowballs[i].texture) {
            SDL_RenderCopy(renderer, snowballs[i].texture, NULL, &snowballs[i].rect);
        }
    }
}


void checkSnowballEnemyCollision(Snowball snowballs[], int *snowballCount, Enemy *enemy, HUD *hud) {
    for (int i = 0; i < *snowballCount; i++) {
        if (snowballs[i].active && SDL_HasIntersection(&snowballs[i].rect, &enemy->position)) {
            enemy->state.hurt = 1;  
            snowballs[i].active = 0; 

       
            if (hud->trees > 0) {
                hud->trees--;
            }

        
            if (hud->trees <= 0) {
                enemy->state.neutralized = 1;
                enemy->state.alive = 0;
            }
        }
    }
}


void initPlayer(SDL_Renderer *renderer, Player *player, const char *spritePath, int nbCols, int nbRows, Enemy *enemy) {
    SDL_Surface *surface = IMG_Load(spritePath);
    if (!surface) return;
    player->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    int imgW, imgH;
    SDL_QueryTexture(player->texture, NULL, NULL, &imgW, &imgH);
    player->frameWidth  = imgW / nbCols;
    player->frameHeight = imgH / nbRows;
    player->maxFrames   = nbCols;
    player->frame       = 0;
    player->isRunning   = 0;   
    player->speed       = 3;   
    player->direction   = enemy->direction;

   
    player->position.x = 0;


    player->position.y = enemy->position.y;

    player->position.w = player->frameWidth;
    player->position.h = player->frameHeight;
}


void handlePlayerMovement(SDL_Event *event, Player *player) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_RIGHT:
                player->isRunning = 1;      
                player->speed = 3;           
                player->direction = 0;       
                break;
            case SDLK_LEFT:
                player->isRunning = 1;      
                player->speed = -3;         
                player->direction = 1;       
                break;
        }
    }
    else if (event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym) {
            case SDLK_RIGHT:
            case SDLK_LEFT:
                player->isRunning = 0;       
                player->speed = 0;
                break;
        }
    }
}


void updatePlayer(Player *player, Enemy *enemy) {
    if (player->isRunning) {
        player->frame = (player->frame + 1) % player->maxFrames;
        player->position.x += player->speed;
    }

  
    int distanceMin = 0; 
    if (player->position.x + player->position.w >= enemy->position.x - distanceMin) {
        player->position.x = enemy->position.x - distanceMin - player->position.w;
    }

   
    if (player->position.x < 0) {
        player->position.x = 0;
    }
}

void renderPlayer(SDL_Renderer *renderer, Player *player) {
    SDL_Rect src = {
        player->frame * player->frameWidth,         
        player->direction * player->frameHeight,     
        player->frameWidth,
        player->frameHeight
    };
    SDL_RenderCopy(renderer, player->texture, &src, &player->position);
}


void checkPlayerEnemyCollision(Player *player, Enemy *enemy) {
    if (SDL_HasIntersection(&player->position, &enemy->position)) {
        enemy->state.playerTouch = 1; 
    }
}


void destroyAll(Enemy *enemy, Obstacle *spider, Obstacle *falling,
                Mix_Chunk *collisionSound, HUD *hud, Snowball *snowball) {
   
    if (enemy->textureNormal) SDL_DestroyTexture(enemy->textureNormal);
    if (enemy->textureHurt)   SDL_DestroyTexture(enemy->textureHurt);

  
    if (spider->texture) SDL_DestroyTexture(spider->texture);
    if (falling->texture) SDL_DestroyTexture(falling->texture);

   
    if (hud->treeTexture) SDL_DestroyTexture(hud->treeTexture);

 
    if (snowball->texture) SDL_DestroyTexture(snowball->texture);

 
    if (collisionSound) Mix_FreeChunk(collisionSound);


    Mix_CloseAudio();
}
