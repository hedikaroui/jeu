#include "game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>

void init(SDL_Renderer *renderer, Enemy *enemy, char *normalPath, int normalCols, int normalRows, char *hurtPath, int hurtCols, int hurtRows, char *treePath, Obstacle *obs, char *obsPath, int x, int y, Snowball *sb, char *snowPath, Player *player, char *playerPath, int nbCols, int nbRows) {
    SDL_Surface *surf;
    surf = IMG_Load(normalPath);
    if (surf) {
        enemy->textureNormal = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        enemy->textureNormal = NULL;
    }
    if (enemy->textureNormal) {
        SDL_QueryTexture(enemy->textureNormal, NULL, NULL, &enemy->frameWidth, &enemy->frameHeight);
        enemy->frameWidth /= normalCols;
        enemy->frameHeight /= normalRows;
    }
    surf = IMG_Load(hurtPath);
    if (surf) {
        enemy->textureHurt = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        enemy->textureHurt = NULL;
    }
    enemy->position.x = 300;
    enemy->position.y = 260;
    enemy->position.w = enemy->frameWidth;
    enemy->position.h = enemy->frameHeight;
    enemy->posSprite.x = 0;
    enemy->posSprite.y = 0;
    enemy->posSprite.w = enemy->frameWidth;
    enemy->posSprite.h = enemy->frameHeight;
    enemy->frame = 0;
    enemy->nbCols = normalCols;
    enemy->nbRows = normalRows;
    enemy->direction = 0;
    enemy->health = 100;
    enemy->alive = 1;
    enemy->hurt = 0;
    enemy->neutralized = 0;
    enemy->playerTouch = 0;
    enemy->hurtTimer = 0;
    enemy->playerTouchTimer = 0;
    enemy->recoilTimer = 0;
    enemy->recoiling = 0;
    enemy->blinkTimer = 0;
    enemy->blinking = 0;
    enemy->trees = 7;
    enemy->currentSpeed = 2;
    surf = IMG_Load(treePath);
    if (surf) {
        enemy->treeTexture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        enemy->treeTexture = NULL;
    }
    surf = IMG_Load(obsPath);
    if (surf) {
        obs->texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        obs->texture = NULL;
    }
    obs->position.x = x;
    obs->position.y = y;
    obs->position.w = 128;
    obs->position.h = 128;
    obs->active = 1;
    surf = IMG_Load(snowPath);
    if (surf) {
        sb->texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        sb->texture = NULL;
    }
    sb->rect.w = 32;
    sb->rect.h = 32;
    sb->x = 0;
    sb->y = 0;
    sb->speed = 0;
    sb->active = 0;
    surf = IMG_Load(playerPath);
    if (surf) {
        player->texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        player->texture = NULL;
    }
    int imgW, imgH;
    if (player->texture) {
        SDL_QueryTexture(player->texture, NULL, NULL, &imgW, &imgH);
        player->frameWidth = imgW / nbCols;
        player->frameHeight = imgH / nbRows;
    } else {
        player->frameWidth = 0;
        player->frameHeight = 0;
    }
    player->maxFrames = nbCols;
    player->frame = 0;
    player->isRunning = 0;
    player->speed = 0;
    player->direction = 0;
    player->position.x = 0;
    player->position.y = enemy->position.y;
    player->position.w = player->frameWidth;
    player->position.h = player->frameHeight;
    player->acceleration = 1;
    player->maxSpeed = 999;
}
void move(Enemy *enemy, Obstacle *obs) {
    if (!enemy->alive) return;
    if (enemy->trees > 4) {
        enemy->currentSpeed = 5;  // Augmentée de 2 à 5 pour meilleure progression
    } else {
        if (enemy->currentSpeed < 20) enemy->currentSpeed += 2;
    }
    enemy->direction = 0;
    enemy->position.x += enemy->currentSpeed;
    if (enemy->position.x > 800) {
        enemy->position.x = -enemy->position.w;
    }
    enemy->frame++;
    if (enemy->frame >= enemy->nbCols) enemy->frame = 0;
    enemy->posSprite.x = enemy->frame * enemy->frameWidth;
    enemy->posSprite.y = enemy->direction * enemy->frameHeight;
    if (enemy->hurt) {
        enemy->hurtTimer++;
        if (enemy->hurtTimer > 60) {
            enemy->hurt = 0;
            enemy->hurtTimer = 0;
        }
    }
    if (enemy->blinking) {
        enemy->blinkTimer++;
        if (enemy->blinkTimer > 40) {
            enemy->blinking = 0;
            enemy->blinkTimer = 0;
        }
    }
    if (enemy->playerTouch) {
        enemy->playerTouchTimer++;
        if (enemy->playerTouchTimer > 20) {
            enemy->playerTouch = 0;
            enemy->playerTouchTimer = 0;
        }
    }
    if (obs->active) {
        obs->position.x -= 3;
        if (obs->position.x < -obs->position.w) {
            obs->position.x = 800;
        }
    }
}
void updatePlayer(Player *player, Enemy *enemy, Snowball snowballs[], int *snowballCount) {
    // Le joueur bouge selon sa propre vitesse (contrôlé par les flèches)
    if (!player->isRunning) {
        if (player->speed > 0) player->speed--;
        else if (player->speed < 0) player->speed++;
    }
    
    if (player->isRunning || player->speed != 0) {
        player->frame = (player->frame + 1) % player->maxFrames;
        player->position.x += player->speed;
    }
    
    // Limite gauche (début de la fenêtre)
    if (player->position.x < 0) {
        player->position.x = 0;
    }
    
    // Le joueur ne doit jamais dépasser l'ennemi (rester toujours derrière)
    // Avec une marge de 200 pixels pour permettre le mouvement libre
    if (player->position.x + player->position.w > enemy->position.x - 200) {
        int targetX = enemy->position.x - player->position.w - 200;
        // Vérifier que la position cible n'est pas négative
        if (targetX >= 0) {
            player->position.x = targetX;
            player->speed = 0;  // Arrête le mouvement si on s'approche trop
        }
    }
    
    // Limite droite (bord de la fenêtre, sécurité)
    if (player->position.x + player->position.w > 800) {
        player->position.x = 800 - player->position.w;
    }
    
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
void affichage(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Snowball snowballs[], int snowballCount, Player *player) {
    if (enemy->alive && enemy->textureNormal) {
        if (enemy->blinking) {
            if ((enemy->blinkTimer / 5) % 2 == 0 && enemy->textureHurt) {
                SDL_SetTextureColorMod(enemy->textureHurt, 255, 80, 80);
                SDL_SetTextureAlphaMod(enemy->textureHurt, 210);
                SDL_RenderCopy(renderer, enemy->textureHurt, &enemy->posSprite, &enemy->position);
            }
        } else if (enemy->hurt && enemy->textureHurt) {
            SDL_SetTextureColorMod(enemy->textureHurt, 255, 80, 80);
            SDL_SetTextureAlphaMod(enemy->textureHurt, 210);
            SDL_RenderCopy(renderer, enemy->textureHurt, &enemy->posSprite, &enemy->position);
        } else if (enemy->playerTouch) {
            SDL_SetTextureColorMod(enemy->textureNormal, 255, 80, 80);
            SDL_SetTextureAlphaMod(enemy->textureNormal, 210);
            SDL_RenderCopy(renderer, enemy->textureNormal, &enemy->posSprite, &enemy->position);
        } else {
            SDL_SetTextureColorMod(enemy->textureNormal, 255, 255, 255);
            SDL_SetTextureAlphaMod(enemy->textureNormal, 255);
            SDL_RenderCopy(renderer, enemy->textureNormal, &enemy->posSprite, &enemy->position);
        }
    }
    if (obs->active && obs->texture) {
        SDL_RenderCopy(renderer, obs->texture, NULL, &obs->position);
    }
    for (int i = 0; i < enemy->trees; i++) {
        if (enemy->treeTexture) {
            SDL_Rect dst = {10 + i * 75, 10, 65, 65};
            SDL_RenderCopy(renderer, enemy->treeTexture, NULL, &dst);
        }
    }
    for (int i = 0; i < snowballCount; i++) {
        if (snowballs[i].active && snowballs[i].texture) {
            SDL_RenderCopy(renderer, snowballs[i].texture, NULL, &snowballs[i].rect);
        }
    }
    if (player->texture) {
        SDL_Rect src = {player->frame * player->frameWidth, player->direction * player->frameHeight, player->frameWidth, player->frameHeight};
        SDL_RenderCopy(renderer, player->texture, &src, &player->position);
    }
}
void checkCollision(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound, Snowball snowballs[], int *snowballCount, Player *player) {
    if (!enemy->alive) return;
    if (obs->active && SDL_HasIntersection(&enemy->position, &obs->position)) {
        if (collisionSound) Mix_PlayChannel(-1, collisionSound, 0);
    }
    for (int i = 0; i < *snowballCount; i++) {
        if (snowballs[i].active && SDL_HasIntersection(&snowballs[i].rect, &enemy->position)) {
            enemy->blinking = 1;
            enemy->blinkTimer = 0;
            snowballs[i].active = 0;
            SDL_Surface *surf = IMG_Load("enemy2.png");
            if (surf) {
                if (enemy->textureHurt) SDL_DestroyTexture(enemy->textureHurt);
                enemy->textureHurt = SDL_CreateTextureFromSurface(renderer, surf);
                SDL_FreeSurface(surf);
            }
            if (enemy->trees > 0) enemy->trees--;
            if (enemy->trees <= 0) {
                enemy->neutralized = 1;
                enemy->alive = 0;
            }
        }
    }
    if (SDL_HasIntersection(&player->position, &enemy->position)) {
        enemy->playerTouch = 1;
        enemy->playerTouchTimer = 0;
    }
}
void handleInput(SDL_Event *event, Player *player, Snowball snowballs[], int *snowballCount) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_RIGHT:
                player->isRunning = 1;
                if (player->speed < player->maxSpeed) player->speed += player->acceleration;
                player->direction = 0;
                break;
            case SDLK_LEFT:
                player->isRunning = 1;
                if (player->speed > -player->maxSpeed) player->speed -= player->acceleration;
                player->direction = 1;
                break;
            case SDLK_s:
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
                break;
        }
    } else if (event->type == SDL_KEYUP) {
        if (event->key.keysym.sym == SDLK_RIGHT || event->key.keysym.sym == SDLK_LEFT) {
            player->isRunning = 0;
        }
    }
}
void initObstacle(SDL_Renderer *renderer, Obstacle *obs, char *path, int x, int y) {
    SDL_Surface *surface = IMG_Load(path);
    if (surface) {
        if (obs->texture) SDL_DestroyTexture(obs->texture);
        obs->texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    } else {
        obs->texture = NULL;
    }
    obs->position.x = x;
    obs->position.y = y;
    obs->position.w = 128;
    obs->position.h = 128;
    obs->active = 1;
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
            if (background) SDL_DestroyTexture(background);
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
void destroyAll(Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound, Snowball *snowball) {
    if (enemy->textureNormal) SDL_DestroyTexture(enemy->textureNormal);
    if (enemy->textureHurt) SDL_DestroyTexture(enemy->textureHurt);
    if (enemy->treeTexture) SDL_DestroyTexture(enemy->treeTexture);
    if (obs->texture) SDL_DestroyTexture(obs->texture);
    if (snowball->texture) SDL_DestroyTexture(snowball->texture);
    if (collisionSound) Mix_FreeChunk(collisionSound);
    Mix_CloseAudio();
}
