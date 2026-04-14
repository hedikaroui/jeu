#include "enemi.h"
#include "bg.h"
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
void renderEnemy(SDL_Renderer *renderer, Enemy *enemy, SDL_Rect *cam) {
    if (!enemy->state.alive) return;

    SDL_Rect dst = enemy->position;
    if (cam) {
        dst.x -= cam->x;
        dst.y -= cam->y;
    }

    if (enemy->state.hurt) {
        SDL_SetTextureColorMod(enemy->textureHurt, 255, 0, 0);
        SDL_SetTextureAlphaMod(enemy->textureHurt, 180);
        SDL_RenderCopy(renderer, enemy->textureHurt, &enemy->posSprite, &dst);
    } else if (enemy->state.playerTouch) {
        SDL_SetTextureColorMod(enemy->textureNormal, 255, 0, 0);
        SDL_SetTextureAlphaMod(enemy->textureNormal, 180);
        SDL_RenderCopy(renderer, enemy->textureNormal, &enemy->posSprite, &dst);
    } else {
        SDL_SetTextureColorMod(enemy->textureNormal, 255, 255, 255);
        SDL_SetTextureAlphaMod(enemy->textureNormal, 255);
        SDL_RenderCopy(renderer, enemy->textureNormal, &enemy->posSprite, &dst);
    }
}
//
// ================= MOVE & ANIMATE ENEMY =================
void moveAndAnimateEnemy(Enemy *enemy) {
    static int recoiling = 0;
    static int recoilTimer = 0;
    static int hurtTimer = 0;
    static int playerTouchTimer = 0;

    if (enemy->health > 4) {
        // Mode normal : avance lente (droite)
        enemy->direction = 0; // 0 = droite
        enemy->position.x += 1;
    } else {
        // 🔥 Mode spécial (vie <= 4)
        if (!recoiling && recoilTimer == 0) {
            recoiling = 1;
            recoilTimer = 40; // recule pendant 40 frames
        }

        if (recoiling) {
            // recule en arrière avec marche gauche
            enemy->direction = 1; // 1 = gauche
            enemy->position.x -= 8;
            recoilTimer--;
            if (recoilTimer <= 0) {
                recoiling = 0; // fin recule
            }
        } else {
            // course rapide en avant (droite)
            enemy->direction = 0;
            enemy->position.x += 15;
        }
    }

    // 🔄 Avance infinie : quand il dépasse la largeur de niveau, il repart du début
    if (enemy->position.x > LEVEL_WIDTH) {
        enemy->position.x = -enemy->position.w;
    }

    // Animation frame
    enemy->frame++;
    if (enemy->frame >= enemy->nbCols) enemy->frame = 0;
    enemy->posSprite.x = enemy->frame * enemy->frameWidth;

    // Choix spritesheet selon direction
    enemy->posSprite.y = enemy->direction * enemy->frameHeight;

    // Timer blessé
    if (enemy->state.hurt) {
        hurtTimer++;
        if (hurtTimer > 70) {
            enemy->state.hurt = 0;
            hurtTimer = 0;
        }
    }

    // Timer touché par joueur
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
    obs->texture = NULL;
    obs->active = 0;
    SDL_Surface *surface = IMG_Load(spritePath);
    if (!surface) return;
    obs->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    obs->position.x = x;
    obs->position.y = y;
    obs->position.w = 128;
    obs->position.h = 128;
    obs->active = (obs->texture != NULL) ? 1 : 0;
}

// 
void renderObstacle(SDL_Renderer *renderer, Obstacle *obs) {
    if (!obs || !obs->active) return;
    if (obs->texture)
        SDL_RenderCopy(renderer, obs->texture, NULL, &obs->position);
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
        if (currentBg > 4) currentBg = 1; // 🔥 maintenant 4 backgrounds

        char filename[30];
        sprintf(filename, "background%d.png", currentBg);

        SDL_Surface *surface = IMG_Load(filename);
        if (surface) {
            SDL_DestroyTexture(background);
            background = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }

        enemy->position.x = 0;

        // 🔥 ajout du 4e background
        switch (currentBg) {
            case 1: initObstacle(renderer, obs, "falling.png", 600, 400); break;
            case 2: initObstacle(renderer, obs, "spider.png", 600, 400); break;
            case 3: initObstacle(renderer, obs, "falling.png", 600, 400); break;
            case 4: initObstacle(renderer, obs, "spider.png", 600, 400); break; // exemple pour le 4e
        }
    }

    if (background) {
        SDL_Rect bgRect = {0, 0, 800, 600};
        SDL_RenderCopy(renderer, background, NULL, &bgRect);
    }
}
// ================= HUD =================

void initHUD(HUD *hud, SDL_Renderer *renderer, const char *treePath) {
    hud->trees = 7; // départ avec 7 sapins
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

// ================= SNOWBALL =================
void initSnowball(SDL_Renderer *renderer, Snowball *sb, const char *spritePath) {
    sb->texture = NULL;
    // 🔥 Taille plus grande et visible par défaut
    sb->rect.w = 32;   // largeur fixe
    sb->rect.h = 32;   // hauteur fixe

    sb->x = sb->y = 0;
    sb->speed = 0;
    sb->active = 0;
    sb->rect.x = 0;
    sb->rect.y = 0;

    SDL_Surface *surface = IMG_Load(spritePath);
    if (!surface) return;
    sb->texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

// ================= SNOWBALL THROW =================
void handleSnowballThrow(SDL_Event *event, EPlayer *player, Snowball snowballs[], int *snowballCount) {
    // Use Space to throw snowballs to avoid conflict with 'S' movement
    if (event->type == SDL_KEYDOWN && (event->key.keysym.sym == SDLK_SPACE || event->key.keysym.scancode == SDL_SCANCODE_SPACE)) {
        if (*snowballCount < MAX_SNOWBALLS) {
            Snowball *sb = &snowballs[*snowballCount];
            sb->x = player->position.x + player->position.w; // sort du joueur
            sb->y = player->position.y + (player->position.h / 2) - (sb->rect.h / 2); // centré verticalement
            sb->speed = 8;   // vitesse fixe
            sb->active = 1;
            sb->rect.x = sb->x;
            sb->rect.y = sb->y;
            sb->texture = snowballs[0].texture; // partager la même texture
            (*snowballCount)++;
        }
    }
}

// ================= SNOWBALL UPDATE =================
void updateSnowballs(Snowball snowballs[], int *snowballCount) {
    if (!snowballCount) return;
    int count = *snowballCount;
    if (count <= 0) return;
    if (count > MAX_SNOWBALLS) count = MAX_SNOWBALLS;
    for (int i = 0; i < count; i++) {
        if (snowballs[i].active) {
            snowballs[i].x += snowballs[i].speed;
            snowballs[i].rect.x = snowballs[i].x;

            // 🔥 désactiver si sort de l’écran
            if (snowballs[i].x > 800) {
                snowballs[i].active = 0;
            }
        }
    }
}

// ================= SNOWBALL RENDER =================
void renderSnowballs(SDL_Renderer *renderer, Snowball snowballs[], int snowballCount) {
    if (snowballCount <= 0) return;
    if (snowballCount > MAX_SNOWBALLS) snowballCount = MAX_SNOWBALLS;
    for (int i = 0; i < snowballCount; i++) {
        if (snowballs[i].active) {
            void *texptr = (void*)snowballs[i].texture;
            fprintf(stderr, "DEBUG SNOW i=%d active=%d tex=%p rect=(%d,%d,%d,%d)\n", i, snowballs[i].active, texptr, snowballs[i].rect.x, snowballs[i].rect.y, snowballs[i].rect.w, snowballs[i].rect.h);
            if (snowballs[i].texture) {
                SDL_RenderCopy(renderer, snowballs[i].texture, NULL, &snowballs[i].rect);
            }
        }
    }
}

// ================= SNOWBALL COLLISION =================
void checkSnowballEnemyCollision(Snowball snowballs[], int *snowballCount, Enemy *enemy, HUD *hud) {
    for (int i = 0; i < *snowballCount; i++) {
        if (snowballs[i].active && SDL_HasIntersection(&snowballs[i].rect, &enemy->position)) {
            enemy->state.hurt = 1;   // ennemi clignote
            snowballs[i].active = 0; // 🔥 boule disparaît après impact

            // Exemple : décrémenter HUD
            if (hud->trees > 0) {
                hud->trees--;
            }

            // Si plus de "trees" → ennemi neutralisé
            if (hud->trees <= 0) {
                enemy->state.neutralized = 1;
                enemy->state.alive = 0;
            }
        }
    }
}

// ================= PLAYER =================
void initPlayer(SDL_Renderer *renderer, EPlayer *player, const char *spritePath, int nbCols, int nbRows, Enemy *enemy) {
    player->texture = NULL;
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
    player->isRunning   = 0;   // 🔥 au début il ne bouge pas
    player->speed       = 3;   // vitesse de base
    player->direction   = enemy->direction; // 🔥 même orientation que l’ennemi

    // 🔥 Position initiale : tout à gauche de la fenêtre
    player->position.x = 0;

    // 🔥 Même ligne verticale que l’ennemi
    player->position.y = enemy->position.y;

    player->position.w = player->frameWidth;
    player->position.h = player->frameHeight;
}

// ================= PLAYER MOVEMENT =================
void handlePlayerMovement(SDL_Event *event, EPlayer *player) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_RIGHT:
                player->isRunning = 1;       // commence à courir
                player->speed = 3;           // vitesse
                player->direction = 0;       // ligne droite du spritesheet
                break;
            case SDLK_LEFT:
                player->isRunning = 1;       // recule
                player->speed = -3;          // vitesse négative
                player->direction = 1;       // ligne gauche du spritesheet
                break;
        }
    }
    else if (event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym) {
            case SDLK_RIGHT:
            case SDLK_LEFT:
                player->isRunning = 0;       // 🔥 s’arrête quand on relâche
                player->speed = 0;
                break;
        }
    }
}

// ================= PLAYER UPDATE =================
void updatePlayer(EPlayer *player, Enemy *enemy) {
    if (player->isRunning) {
        player->frame = (player->frame + 1) % player->maxFrames;
        player->position.x += player->speed;
    }

    // 🔥 Toujours garder le joueur derrière l’ennemi
    int distanceMin = 0; // autoriser à toucher l’ennemi
    if (player->position.x + player->position.w >= enemy->position.x - distanceMin) {
        player->position.x = enemy->position.x - distanceMin - player->position.w;
    }

    // 🔥 Empêcher de sortir de l’écran
    if (player->position.x < 0) {
        player->position.x = 0;
    }
}
// ================= PLAYER RENDER =================
void renderPlayer(SDL_Renderer *renderer, EPlayer *player) {
    if (!player || !player->texture) return;
    SDL_Rect src = {
        player->frame * player->frameWidth,          // colonne (animation)
        player->direction * player->frameHeight,     // ligne (0 = droite, 1 = gauche)
        player->frameWidth,
        player->frameHeight
    };
    SDL_RenderCopy(renderer, player->texture, &src, &player->position);
}

// ================= PLAYER COLLISION =================
void checkPlayerEnemyCollision(EPlayer *player, Enemy *enemy) {
    if (SDL_HasIntersection(&player->position, &enemy->position)) {
        enemy->state.playerTouch = 1; // ennemi clignote si joueur le touche
    }
}
// ================= CLEANUP =================
// Libère toutes les ressources (textures, sons, audio)
void destroyAll(Enemy *enemy, Obstacle *spider, Obstacle *falling,
                Mix_Chunk *collisionSound, HUD *hud, Snowball *snowball) {
    // Libération textures Enemy
    if (enemy->textureNormal) SDL_DestroyTexture(enemy->textureNormal);
    if (enemy->textureHurt)   SDL_DestroyTexture(enemy->textureHurt);

    // Libération textures Obstacle
    if (spider->texture) SDL_DestroyTexture(spider->texture);
    if (falling->texture) SDL_DestroyTexture(falling->texture);

    // Libération texture HUD
    if (hud->treeTexture) SDL_DestroyTexture(hud->treeTexture);

    // Libération texture Snowball (une seule image partagée)
    if (snowball->texture) SDL_DestroyTexture(snowball->texture);

    // Libération sons
    if (collisionSound) Mix_FreeChunk(collisionSound);

    // Fermeture audio
    Mix_CloseAudio();
}

