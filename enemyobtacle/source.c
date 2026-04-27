#include "game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define OBSTACLE_SIZE 128
#define DIRECTION_RIGHT 0
#define DIRECTION_LEFT 1

// ================= UTILITY =================
// Charger une texture depuis un fichier
SDL_Texture* loadTexture(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    if (!surface) return NULL;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Dessiner une texture avec couleur et alpha
void drawTexture(SDL_Renderer *renderer, SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dst, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    SDL_SetTextureColorMod(texture, r, g, b);
    SDL_SetTextureAlphaMod(texture, a);
    SDL_RenderCopy(renderer, texture, src, dst);
}

static double circleCircumscribedRadius(SDL_Rect rect) {
    double halfWidth = rect.w / 2.0;
    double halfHeight = rect.h / 2.0;
    return sqrt((halfWidth * halfWidth) + (halfHeight * halfHeight));
}

static int collisionTrigonometrique(SDL_Rect rect1, SDL_Rect rect2) {
    double x1 = rect1.x + rect1.w / 2.0;
    double y1 = rect1.y + rect1.h / 2.0;
    double x2 = rect2.x + rect2.w / 2.0;
    double y2 = rect2.y + rect2.h / 2.0;
    double r1 = circleCircumscribedRadius(rect1);
    double r2 = circleCircumscribedRadius(rect2);
    double dx = x1 - x2;
    double dy = y1 - y2;
    double distanceSquared = (dx * dx) + (dy * dy);
    double radiusSum = r1 + r2;

    return distanceSquared <= (radiusSum * radiusSum);
}

static void seedRandomOnce(void) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)SDL_GetTicks());
        seeded = 1;
    }
}

static int clampInt(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

// ================= ENEMY =================
void initEnemy(SDL_Renderer *renderer, Enemy *enemy, const char *normalPath, int normalCols, int normalRows, const char *hurtPath, int hurtCols, int hurtRows) {
    enemy->textureNormal = loadTexture(renderer, normalPath);
    enemy->textureHurt = loadTexture(renderer, hurtPath);

    SDL_QueryTexture(enemy->textureNormal, NULL, NULL, &enemy->frameWidth, &enemy->frameHeight);
    enemy->frameWidth /= normalCols;
    enemy->frameHeight /= normalRows;

    enemy->position = (SDL_Rect){300, 260, enemy->frameWidth, enemy->frameHeight};
    enemy->posSprite = (SDL_Rect){0, 0, enemy->frameWidth, enemy->frameHeight};
    enemy->frame = 0;
    enemy->nbCols = normalCols;
    enemy->nbRows = normalRows;
    enemy->direction = 0;
    enemy->health = 100;
    enemy->state.alive = 1;
    enemy->state.hurt = 0;
    enemy->state.neutralized = 0;
}

void renderEnemy(SDL_Renderer *renderer, Enemy *enemy) {
    if (enemy->state.alive) {
        SDL_Texture *texture = enemy->state.hurt ? enemy->textureHurt : enemy->textureNormal;
        drawTexture(renderer, texture, &enemy->posSprite, &enemy->position, 255, 0, 0, 180);
    }
}

void moveAndAnimateEnemy(Enemy *enemy, HUD *hud) {
    static int hurtTimer = 0;
    int speed = 2;
    
    // Courir illimité vers l'avant (droite)
    enemy->direction = 0;
    
    // Accélérer si HUD < 4
    if (hud->trees < 4) {
        speed = 5;
    }
    
    enemy->position.x += speed;

    // Boucle infinie : quand il dépasse l'écran, repart du début
    if (enemy->position.x > 800) {
        enemy->position.x = -enemy->position.w;
    }

    // Animation frame
    enemy->frame++;
    if (enemy->frame >= enemy->nbCols) enemy->frame = 0;
    enemy->posSprite.x = enemy->frame * enemy->frameWidth;

    // Spritesheet selon direction
    enemy->posSprite.y = enemy->direction * enemy->frameHeight;

    // Timer blessé
    if (enemy->state.hurt) {
        hurtTimer++;
        if (hurtTimer > 70) {
            enemy->state.hurt = 0;
            hurtTimer = 0;
        }
    }
}

// Vérifier collision avec l'ennemi (diminue le HUD et charge texture hurt)
void checkEnemyHit(SDL_Renderer *renderer, Enemy *enemy, HUD *hud, const char *hurtPath) {
    // Charger la texture hurt si elle n'existe pas
    if (!enemy->textureHurt) {
        SDL_Surface *surface = IMG_Load(hurtPath);
        if (surface) {
            enemy->textureHurt = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }
    
    // Quand quelque chose touche l'ennemi, diminuer HUD
    if (hud->trees > 0) {
        hud->trees--;
    }
    enemy->state.hurt = 1;
    
    // Ennemi neutralisé si plus de trees
    if (hud->trees <= 0) {
        enemy->state.neutralized = 1;
        enemy->state.alive = 0;
    }
}

// ================= OBSTACLE =================
void initObstacle(SDL_Renderer *renderer, Obstacle *obs, const char *spritePath, int x, int y) {
    seedRandomOnce();

    obs->texture = loadTexture(renderer, spritePath);
    obs->position = (SDL_Rect){x, y, OBSTACLE_SIZE, OBSTACLE_SIZE};
    obs->active = 1;
    obs->colliding = 0;
    obs->direction = rand() % 2;
    obs->minX = 0;
    obs->maxX = SCREEN_WIDTH - obs->position.w;
    obs->speed = 1 + rand() % 3;
    obs->baseY = y;
    obs->amplitude = 20.0 + (double)(rand() % 41);
    obs->frequency = 0.015 + ((double)(rand() % 21) / 1000.0);
    obs->phase = (double)(rand() % 628) / 100.0;
}

void renderObstacle(SDL_Renderer *renderer, Obstacle *obs) {
    if (obs->active) {
        SDL_RenderCopy(renderer, obs->texture, NULL, &obs->position);
    }
}

void moveObstacle(Obstacle *obs) {
    if (!obs->active) return;

    if (obs->position.x >= obs->maxX) {
        obs->direction = DIRECTION_LEFT;
    }
    if (obs->position.x <= obs->minX) {
        obs->direction = DIRECTION_RIGHT;
    }

    if (obs->direction == DIRECTION_RIGHT) {
        obs->position.x += obs->speed;
    } else {
        obs->position.x -= obs->speed;
    }

    obs->position.x = clampInt(obs->position.x, obs->minX, obs->maxX);
    obs->position.y = obs->baseY + (int)lround(sin((double)obs->position.x * obs->frequency + obs->phase) * obs->amplitude);
    obs->position.y = clampInt(obs->position.y, 0, SCREEN_HEIGHT - obs->position.h);
}

void checkCollision(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound) {
    (void)renderer;

    if (!enemy || !obs || !obs->active || !enemy->state.alive) return;

    if (collisionTrigonometrique(enemy->position, obs->position)) {
        if (!obs->colliding && collisionSound) {
            Mix_PlayChannel(-1, collisionSound, 0);
        }
        obs->colliding = 1;
    } else {
        obs->colliding = 0;
    }
}

// ================= BACKGROUND =================
void initBackground(HUD *hud, SDL_Renderer *renderer, const char *bgPath) {
    hud->backgroundTexture = loadTexture(renderer, bgPath);
}

void renderBackground(SDL_Renderer *renderer, HUD *hud) {
    if (hud->backgroundTexture) {
        SDL_Rect bgRect = {0, 0, 800, 600};
        SDL_RenderCopy(renderer, hud->backgroundTexture, NULL, &bgRect);
    }
}

// ================= HUD =================
void initHUD(HUD *hud, SDL_Renderer *renderer, const char *treePath) {
    hud->trees = 7;
    hud->treeTexture = loadTexture(renderer, treePath);
}

void renderHUD(SDL_Renderer *renderer, HUD *hud) {
    for (int i = 0; i < hud->trees; i++) {
        SDL_Rect dst = {10 + i*80, 10, 80, 80};
        SDL_RenderCopy(renderer, hud->treeTexture, NULL, &dst);
    }
}

// ================= CLEANUP =================
void destroyAll(Enemy *enemy, Obstacle *spider, Obstacle *falling,
                Mix_Chunk *collisionSound, HUD *hud) {
    if (enemy->textureNormal) SDL_DestroyTexture(enemy->textureNormal);
    if (enemy->textureHurt)   SDL_DestroyTexture(enemy->textureHurt);

    if (spider->texture) SDL_DestroyTexture(spider->texture);
    if (falling->texture) SDL_DestroyTexture(falling->texture);

    if (hud->treeTexture) SDL_DestroyTexture(hud->treeTexture);
    if (hud->backgroundTexture) SDL_DestroyTexture(hud->backgroundTexture);

    if (collisionSound) Mix_FreeChunk(collisionSound);

    Mix_CloseAudio();
}

// ================= ENEMY FIGHT MODE =================
void loadEnemyFightMode(SDL_Renderer *renderer, Enemy *enemy, const char *fightSpritePath, int fightCols, int fightRows) {
    if (enemy->textureNormal) SDL_DestroyTexture(enemy->textureNormal);
    enemy->textureNormal = loadTexture(renderer, fightSpritePath);
    
    int totalW, totalH;
    SDL_QueryTexture(enemy->textureNormal, NULL, NULL, &totalW, &totalH);
    enemy->frameWidth = totalW / fightCols;
    enemy->frameHeight = totalH / fightRows;
    enemy->nbCols = fightCols;
    enemy->nbRows = fightRows;
    enemy->frame = 0;
    enemy->direction = 0;
    enemy->position.w = enemy->frameWidth;
    enemy->position.h = enemy->frameHeight;
    enemy->state.hurt = 0;
}
