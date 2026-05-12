/**
 * @file source.c
 * @brief Fichier contenant les fonctions principales du jeu.
 * @author Maram Hnaien
 * @version 1
 * @date 2026
 */
#include "game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define BACKGROUND_COUNT 4
#define PLAYER_FIXED_X 120
#define ENEMY_START_GAP 120

static int gameScrollOffsetX = 0;

static void shiftWorld(int delta, Enemy *enemy, Obstacle *obs,
                       Snowball snowballs[], int snowballCount) {
    if (delta == 0) {
        return;
    }

    if (enemy) {
        enemy->position.x -= delta;
    }
    if (obs && obs->active) {
        obs->position.x -= delta;
    }
    for (int i = 0; i < snowballCount; i++) {
        if (!snowballs[i].active) {
            continue;
        }
        snowballs[i].x -= delta;
        snowballs[i].rect.x = snowballs[i].x;
    }
}

static void applyScrolling(Player *player, Enemy *enemy, Obstacle *obs,
                           Snowball snowballs[], int snowballCount) {
    int delta;

    if (!player) {
        return;
    }

    delta = player->position.x - PLAYER_FIXED_X;
    if (delta == 0) {
        return;
    }

    if (gameScrollOffsetX + delta < 0) {
        delta = -gameScrollOffsetX;
    }

    player->position.x = PLAYER_FIXED_X;
    gameScrollOffsetX += delta;
    shiftWorld(delta, enemy, obs, snowballs, snowballCount);
}

static SDL_Texture *loadBackgroundTexture(SDL_Renderer *renderer, int index) {
    char filename[32];

    if (!renderer) {
        return NULL;
    }

    if (index < 1 || index > BACKGROUND_COUNT) {
        index = 1;
    }

    snprintf(filename, sizeof(filename), "background%d.png", index);
    return loadTexture(renderer, filename);
}
/**
 * @brief Cherche le chemin correct d'un fichier asset.
 * @param baseName Nom de base du fichier à chercher.
 * @param output Chaîne où sera stocké le chemin trouvé.
 * @param outputSize Taille maximale de la chaîne output.
 * @return 1 si le fichier est trouvé, 0 sinon.
 */
int resolveAssetPath(const char *baseName, char *output, size_t outputSize) {
    const char *extensions[] = {"", ".png", ".jpg", ".jpeg", ".bmp"};
    size_t extensionCount = sizeof(extensions) / sizeof(extensions[0]);

    if (!baseName || !output || outputSize == 0) {
        return 0;
    }

    for (size_t i = 0; i < extensionCount; i++) {
        SDL_RWops *file;

        snprintf(output, outputSize, "%s%s", baseName, extensions[i]);
        file = SDL_RWFromFile(output, "rb");
        if (file) {
            SDL_RWclose(file);
            return 1;
        }
    }

    output[0] = '\0';
    return 0;
}
/**
 * @brief Charge une image et la transforme en texture SDL.
 * @param renderer Renderer SDL utilisé pour créer la texture.
 * @param path Chemin de l'image à charger.
 * @return La texture SDL créée, ou NULL en cas d'erreur.
 */
SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface = IMG_Load(path);
    SDL_Texture *texture = NULL;

    if (!surface) {
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}
/**
 * @brief Dessine un rectangle rempli avec une couleur donnée.
 * @param renderer Renderer SDL utilisé pour dessiner.
 * @param rect Rectangle à remplir.
 * @param color Couleur du rectangle.
 * @return Rien.
 */
void drawFilledRect(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}
/**
 * @brief Dessine le contour d'un rectangle.
 * @param renderer Renderer SDL utilisé pour dessiner.
 * @param rect Rectangle dont le contour sera dessiné.
 * @param color Couleur du contour.
 * @return Rien.
 */
void drawRectOutline(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}
/**
 * @brief Crée une texture SDL à partir d'un texte.
 * @param renderer Renderer SDL utilisé pour créer la texture.
 * @param font Police utilisée pour écrire le texte.
 * @param text Texte à transformer en texture.
 * @param color Couleur du texte.
 * @param w Pointeur vers la largeur du texte généré.
 * @param h Pointeur vers la hauteur du texte généré.
 * @return La texture contenant le texte, ou NULL en cas d'erreur.
 */
SDL_Texture *createTextTexture(SDL_Renderer *renderer, TTF_Font *font,
                               const char *text, SDL_Color color,
                               int *w, int *h) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    if (!renderer || !font || !text) {
        return NULL;
    }

    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        return NULL;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (w) *w = surface->w;
    if (h) *h = surface->h;
    SDL_FreeSurface(surface);
    return texture;
}
/**
 * @brief Calcule la position d'une frame dans un spritesheet.
 * @param texture Texture contenant le spritesheet.
 * @param cols Nombre de colonnes dans le spritesheet.
 * @param rows Nombre de lignes dans le spritesheet.
 * @param frameIndex Indice de la frame à récupérer.
 * @return Rectangle correspondant à la frame demandée.
 */
static SDL_Rect getSpriteSheetFrame(SDL_Texture *texture, int cols, int rows, int frameIndex) {
    SDL_Rect frameRect = {0, 0, 0, 0};
    int texW = 0;
    int texH = 0;

    if (!texture || cols <= 0 || rows <= 0) {
        return frameRect;
    }

    SDL_QueryTexture(texture, NULL, NULL, &texW, &texH);
    if (texW <= 0 || texH <= 0) {
        return frameRect;
    }

    frameRect.w = texW / cols;
    frameRect.h = texH / rows;
    frameRect.x = (frameIndex % cols) * frameRect.w;
    frameRect.y = ((frameIndex / cols) % rows) * frameRect.h;

    return frameRect;
}
/**
 * @brief Met à jour la taille du sprite de l'ennemi selon la texture utilisée.
 * @param enemy Pointeur vers l'ennemi.
 * @param texture Texture utilisée pour calculer la taille du sprite.
 * @return Rien.
 */
static void updateEnemySpriteSize(Enemy *enemy, SDL_Texture *texture) {
    int imgW = 0;
    int imgH = 0;

    if (!enemy || !texture) {
        return;
    }

    SDL_QueryTexture(texture, NULL, NULL, &imgW, &imgH);
    if (imgW <= 0 || imgH <= 0 || enemy->nbCols <= 0 || enemy->nbRows <= 0) {
        return;
    }

    enemy->frameWidth = imgW / enemy->nbCols;
    enemy->frameHeight = imgH / enemy->nbRows;
    enemy->position.w = enemy->frameWidth;
    enemy->position.h = enemy->frameHeight;
    enemy->posSprite.w = enemy->frameWidth;
    enemy->posSprite.h = enemy->frameHeight;
}
/**
 * @brief Applique un skin à l'ennemi.
 * @param enemy Pointeur vers l'ennemi.
 * @param skin Pointeur vers le skin choisi.
 * @return Rien.
 */
void setEnemySkin(Enemy *enemy, const Skin *skin) {
    if (!enemy || !skin) {
        return;
    }

    enemy->skinTint = skin->tint;
    enemy->skinName = skin->name;
}
/**
 * @brief Affiche l'écran de sélection aléatoire des skins.
 * @param renderer Renderer SDL utilisé pour l'affichage.
 * @param font Police utilisée pour afficher les textes.
 * @param skins Tableau contenant les skins disponibles.
 * @param skinCount Nombre total de skins disponibles.
 * @return L'indice du skin sélectionné.
 */
int showSkinSelectionScreen(SDL_Renderer *renderer, TTF_Font *font,
                            const Skin skins[], int skinCount) {
    const char *skinNames[] = {
        "Waikiki Draft",
        "Sandveil Prince",
        "Mariviosi",
        "Dragon Alley Kid",
        "Neon Ronin",
        "Vice shadow",
        "Dust Rebellion",
        "Iron Pankster",
        "Kevin"
    };
    SDL_Texture *background = NULL;
    SDL_Texture *previewTextures[MAX_SKINS] = {0};
    SDL_Texture *skinNameTextures[MAX_SKINS] = {0};
    SDL_Texture *skinsLabel = NULL;
    SDL_Texture *nextLabel = NULL;
    SDL_Event event;
    SDL_Point mousePoint = {0, 0};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color selectedColor = {60, 220, 90, 255};
    SDL_Rect buttonRect = {548, 520, 182, 56};
    SDL_Rect skinsLabelRect = {30, 48, 0, 0};
    SDL_Rect nextLabelRect = {0, 0, 0, 0};
    int running = 1;
    int targetIndex;
    int selectedIndex;
    int currentIndex = 0;
    int stepsDone = 0;
    int totalSteps = 0;
    int selectionLocked = 0;
    int previewFrame = 0;
    int displayedFrame = 1;
    Uint32 lastStepTick = 0;
    Uint32 lastPreviewTick = 0;
    char backgroundPath[128];

    if (skinCount <= 0) {
        return 0;
    }

    if (resolveAssetPath("backgroundskin", backgroundPath, sizeof(backgroundPath))) {
        background = loadTexture(renderer, backgroundPath);
    } else {
        background = loadTexture(renderer, "background1.png");
    }

    targetIndex = rand() % skinCount;
    totalSteps = (skinCount * 2) + targetIndex + 1 + (rand() % skinCount);
    selectedIndex = targetIndex;
    lastStepTick = SDL_GetTicks();

    for (int i = 0; i < skinCount && i < MAX_SKINS; i++) {
        previewTextures[i] = loadTexture(renderer, skins[i].normalPath);
    }
    
    for (int i = 0; i < skinCount && i < MAX_SKINS; i++) {
        skinNameTextures[i] = createTextTexture(renderer, font, skinNames[i], white,
                                               NULL, NULL);
    }
    
    skinsLabel = createTextTexture(renderer, font, "Skins", white,
                                   &skinsLabelRect.w, &skinsLabelRect.h);
    nextLabel = createTextTexture(renderer, font, "Next", white,
                                  &nextLabelRect.w, &nextLabelRect.h);
    nextLabelRect.x = buttonRect.x + 62;
    nextLabelRect.y = buttonRect.y + (buttonRect.h - nextLabelRect.h) / 2;
    lastPreviewTick = SDL_GetTicks();
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                       event.button.button == SDL_BUTTON_LEFT) {
                mousePoint.x = event.button.x;
                mousePoint.y = event.button.y;
                if (selectionLocked && SDL_PointInRect(&mousePoint, &buttonRect)) {
                    running = 0;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (selectionLocked &&
                    (event.key.keysym.sym == SDLK_RIGHT ||
                     event.key.keysym.sym == SDLK_RETURN ||
                     event.key.keysym.sym == SDLK_SPACE)) {
                    running = 0;
                }
            }
        }

        if (!selectionLocked && SDL_GetTicks() - lastStepTick >= 120) {
            currentIndex = (currentIndex + 1) % skinCount;
            stepsDone++;
            lastStepTick = SDL_GetTicks();
            if (stepsDone >= totalSteps) {
                currentIndex = targetIndex;
                selectedIndex = targetIndex;
                selectionLocked = 1;
            }
        }
        if (SDL_GetTicks() - lastPreviewTick >= 90) {
            previewFrame = (previewFrame + 1) % 24;
            lastPreviewTick = SDL_GetTicks();
        }
        displayedFrame = previewFrame + 1;
        SDL_GetMouseState(&mousePoint.x, &mousePoint.y);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (background) {
            SDL_Rect bgRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, background, NULL, &bgRect);
        }

        drawFilledRect(renderer, (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, (SDL_Color){4, 10, 18, 110});

        if (skinsLabel) {
            SDL_RenderCopy(renderer, skinsLabel, NULL, &skinsLabelRect);
        }

        for (int i = 0; i < skinCount && i < MAX_SKINS; i++) {
            int row = i / 3;
            int col = i % 3;
            SDL_Rect cell = {14 + (col * 128), 112 + (row * 126), 118, 118};
            SDL_Rect frameRect = getSpriteSheetFrame(previewTextures[i], 5, 5, displayedFrame);

            if (i == currentIndex) {
                SDL_Rect border = {cell.x - 4, cell.y - 4, cell.w + 8, cell.h + 8};
                drawRectOutline(renderer, border, selectedColor);
            }

            if (previewTextures[i] && frameRect.w > 0 && frameRect.h > 0) {
                SDL_RenderCopy(renderer, previewTextures[i], &frameRect, &cell);
            }
        }

        if (previewTextures[currentIndex]) {
            SDL_Rect dst = {446, 98, 286, 286};
            SDL_Rect frameRect = getSpriteSheetFrame(previewTextures[currentIndex], 5, 5, displayedFrame);

            if (frameRect.w > 0 && frameRect.h > 0) {
                double scaleX = (double)dst.w / (double)frameRect.w;
                double scaleY = (double)dst.h / (double)frameRect.h;
                double scale = scaleX < scaleY ? scaleX : scaleY;
                SDL_Rect fitted = {
                    dst.x + (dst.w - (int)(frameRect.w * scale)) / 2,
                    dst.y + (dst.h - (int)(frameRect.h * scale)) / 2,
                    (int)(frameRect.w * scale),
                    (int)(frameRect.h * scale)
                };
                SDL_SetTextureColorMod(previewTextures[currentIndex], 255, 255, 255);
                SDL_RenderCopy(renderer, previewTextures[currentIndex], &frameRect, &fitted);
            }
            
            if (skinNameTextures[currentIndex]) {
                SDL_Rect nameRect = {0, 0, 0, 0};
                SDL_QueryTexture(skinNameTextures[currentIndex], NULL, NULL, &nameRect.w, &nameRect.h);
                nameRect.x = dst.x + (dst.w - nameRect.w) / 2;
                nameRect.y = dst.y + dst.h + 8;
                SDL_RenderCopy(renderer, skinNameTextures[currentIndex], NULL, &nameRect);
            }
        }

        SDL_SetRenderDrawColor(renderer,
                               selectionLocked ? 255 : 170,
                               selectionLocked ? 255 : 170,
                               selectionLocked ? 255 : 170,
                               255);
        SDL_RenderDrawLine(renderer, buttonRect.x + 18, buttonRect.y + (buttonRect.h / 2),
                           buttonRect.x + 50, buttonRect.y + (buttonRect.h / 2));
        SDL_RenderDrawLine(renderer, buttonRect.x + 50, buttonRect.y + (buttonRect.h / 2),
                           buttonRect.x + 38, buttonRect.y + 12);
        SDL_RenderDrawLine(renderer, buttonRect.x + 50, buttonRect.y + (buttonRect.h / 2),
                           buttonRect.x + 38, buttonRect.y + buttonRect.h - 12);

        if (nextLabel) {
            SDL_RenderCopy(renderer, nextLabel, NULL, &nextLabelRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (background) SDL_DestroyTexture(background);
    if (skinsLabel) SDL_DestroyTexture(skinsLabel);
    if (nextLabel) SDL_DestroyTexture(nextLabel);
    for (int i = 0; i < skinCount && i < MAX_SKINS; i++) {
        if (previewTextures[i]) SDL_DestroyTexture(previewTextures[i]);
        if (skinNameTextures[i]) SDL_DestroyTexture(skinNameTextures[i]);
    }

    return selectedIndex;
}
/**
 * @brief Initialise les éléments principaux du jeu.
 * @param renderer Renderer SDL utilisé pour créer les textures.
 * @param enemy Pointeur vers l'ennemi à initialiser.
 * @param normalPath Chemin de l'image normale de l'ennemi.
 * @param normalCols Nombre de colonnes du spritesheet normal.
 * @param normalRows Nombre de lignes du spritesheet normal.
 * @param hurtPath Chemin de l'image de l'ennemi blessé.
 * @param hurtCols Nombre de colonnes du spritesheet blessé.
 * @param hurtRows Nombre de lignes du spritesheet blessé.
 * @param playerTouchPath Chemin de l'image utilisée quand l'ennemi touche le joueur.
 * @param walkPath Chemin du spritesheet de marche.
 * @param runPath Chemin du spritesheet de course.
 * @param treePath Chemin de l'image des arbres.
 * @param obs Pointeur vers l'obstacle à initialiser.
 * @param obsPath Chemin de l'image de l'obstacle.
 * @param x Position horizontale de l'obstacle.
 * @param y Position verticale de l'obstacle.
 * @param sb Pointeur vers la boule de neige à initialiser.
 * @param snowPath Chemin de l'image de la boule de neige.
 * @param player Pointeur vers le joueur à initialiser.
 * @param playerPath Chemin du spritesheet du joueur.
 * @param nbCols Nombre de colonnes du spritesheet du joueur.
 * @param nbRows Nombre de lignes du spritesheet du joueur.
 * @return Rien.
 */
void init(SDL_Renderer *renderer, Enemy *enemy, char *normalPath, int normalCols, int normalRows, char *hurtPath, int hurtCols, int hurtRows, char *playerTouchPath, char *walkPath, char *runPath, char *treePath, Obstacle *obs, char *obsPath, int x, int y, Snowball *sb, char *snowPath, Player *player, char *playerPath, int nbCols, int nbRows) {
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
    surf = IMG_Load(playerTouchPath);
    if (surf) {
        enemy->texturePlayerTouch = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        enemy->texturePlayerTouch = NULL;
    }
    surf = IMG_Load(walkPath);
    if (surf) {
        enemy->textureWalk = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        enemy->textureWalk = NULL;
    }
    surf = IMG_Load(runPath);
    if (surf) {
        enemy->textureRun = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
    } else {
        enemy->textureRun = NULL;
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
    enemy->touchCooldown = 0;
    enemy->recoilTimer = 0;
    enemy->recoiling = 0;
    enemy->blinkTimer = 0;
    enemy->blinking = 0;
    enemy->trees = 7;
    enemy->currentSpeed = 2;
    enemy->startedMoving = 0;
    enemy->animationState = 0;
    enemy->skinTint = (SDL_Color){255, 255, 255, 255};
    enemy->skinName = "Skins";
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
    player->isSprinting = 0;
    player->speed = 0;
    player->direction = 0;
    player->position.x = PLAYER_FIXED_X;
    player->position.y = enemy->position.y;
    player->position.w = player->frameWidth;
    player->position.h = player->frameHeight;
    player->acceleration = 1;
    player->maxSpeed = 22;
    player->shootCooldown = 0;
    
    enemy->position.x = player->position.x + player->position.w + ENEMY_START_GAP;
    gameScrollOffsetX = 0;
    
    enemy->displayDelay = 0;
    obs->displayDelay = rand() % 180 + 60;
}
/**
 * @brief Gère le déplacement et l'animation de l'ennemi.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param player Pointeur vers le joueur.
 * @return Rien.
 */
void move(Enemy *enemy, Obstacle *obs, Player *player) {
    int distance;
    int playerMoving = 0;
    int playerSpeed = 0;
    int maxEnemySpeed = 0;
    int frameDelay = 4;
    int nextAnimation = 0;
    SDL_Texture *nextTexture = NULL;

    if (!enemy->alive) return;
    if (enemy->neutralized) {
        enemy->currentSpeed = 0;
    } else {
        if (player && player->isRunning) {
            playerMoving = 1;
            playerSpeed = abs(player->speed);
            enemy->startedMoving = 1;
        }

        if (player && (playerMoving || enemy->playerTouch) && enemy->displayDelay <= 0) {
            distance = enemy->position.x - (player->position.x + player->position.w);
            if (distance < 0) distance = 0;
            maxEnemySpeed = playerSpeed - 2;
            if (maxEnemySpeed < 1) maxEnemySpeed = 1;
            if (enemy->playerTouch && distance > 260) {
                enemy->playerTouch = 0;
                enemy->playerTouchTimer = 0;
            }

          
            if (enemy->playerTouch) {
                if (enemy->playerTouchTimer < 55) {
                    enemy->currentSpeed = 4 + enemy->playerTouchTimer / 18;
                } else {
                    enemy->currentSpeed = 14 + (enemy->playerTouchTimer - 55) / 5;
                    if (distance < 120) enemy->currentSpeed += 4;
                    if (enemy->currentSpeed > 22) enemy->currentSpeed = 22;
                }
                nextAnimation = 2;
            } else if (playerSpeed <= 3 && distance > 20) {
                enemy->currentSpeed = 2;
                nextAnimation = 1;
            } else if (distance > 100) {
                enemy->currentSpeed = 2 + (180 - distance) / 70;
                if (enemy->currentSpeed < 2) enemy->currentSpeed = 2;
                if (enemy->currentSpeed > 5) enemy->currentSpeed = 5;
                nextAnimation = 1;
            } else {
                enemy->currentSpeed = 8 + (100 - distance) / 18;
                if (enemy->currentSpeed > 16) enemy->currentSpeed = 16;
                nextAnimation = 2;
            }
            if (!enemy->playerTouch && enemy->currentSpeed > maxEnemySpeed) {
                enemy->currentSpeed = maxEnemySpeed;
            }

            enemy->direction = 0;
            enemy->position.x += enemy->currentSpeed;
            if (enemy->position.x < 0) {
                enemy->position.x = 0;
            }

            frameDelay = 5;
            if (nextAnimation == 2) {
                frameDelay = 2;
            }
            enemy->recoilTimer++;
            if (enemy->recoilTimer >= frameDelay) {
                enemy->frame++;
                if (enemy->frame >= enemy->nbCols) enemy->frame = 0;
                enemy->recoilTimer = 0;
            }
        } else {
            enemy->currentSpeed = 0;
            nextAnimation = 0;
            enemy->frame = 0;
            enemy->recoilTimer = 0;
        }

        if (nextAnimation != enemy->animationState) {
            enemy->animationState = nextAnimation;
            enemy->frame = 0;
        }

        if (enemy->animationState == 1 && enemy->textureWalk) {
            nextTexture = enemy->textureWalk;
        } else if (enemy->animationState == 2 && enemy->textureRun) {
            nextTexture = enemy->textureRun;
        } else {
            nextTexture = enemy->textureNormal;
        }

        updateEnemySpriteSize(enemy, nextTexture);
        enemy->posSprite.x = enemy->frame * enemy->frameWidth;
        enemy->posSprite.y = enemy->direction * enemy->frameHeight;
    }
    if (enemy->hurt) {
        enemy->hurtTimer++;
        if (enemy->hurtTimer > 60) {
            enemy->hurt = 0;
            enemy->hurtTimer = 0;
        }
    }
    if (enemy->blinking) {
        enemy->blinkTimer++;
        if (enemy->blinkTimer > 70) {
            enemy->blinking = 0;
            enemy->blinkTimer = 0;
        }
    }
    if (enemy->playerTouch) {
        enemy->playerTouchTimer++;
        if (enemy->playerTouchTimer > 115) {
            enemy->playerTouch = 0;
            enemy->playerTouchTimer = 0;
        }
    }
    if (enemy->touchCooldown > 0) {
        enemy->touchCooldown--;
    }
    if (obs->active) {
        obs->position.x -= 3;
        if (obs->position.x < -obs->position.w) {
            obs->position.x = SCREEN_WIDTH;
            obs->displayDelay = rand() % 180 + 60;
        }
    }
}
/**
 * @brief Met à jour le mouvement, l'animation et les tirs du joueur.
 * @param player Pointeur vers le joueur.
 * @param enemy Pointeur vers l'ennemi.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Pointeur vers le nombre de boules de neige utilisées.
 * @return Rien.
 */
void updatePlayer(Player *player, Enemy *enemy, Obstacle *obs,
                  Snowball snowballs[], int *snowballCount) {
    SDL_PumpEvents();
    Uint8 *keyboard = (Uint8 *)SDL_GetKeyboardState(NULL);
    int movingRight = keyboard[SDL_SCANCODE_RIGHT];
    int movingLeft = keyboard[SDL_SCANCODE_LEFT];
    int runningFast = keyboard[SDL_SCANCODE_R] || keyboard[SDL_SCANCODE_LSHIFT] || keyboard[SDL_SCANCODE_RSHIFT];
    int shooting = keyboard[SDL_SCANCODE_S];
    int targetMaxSpeed;
    int currentAcceleration;

    player->isRunning = movingRight || movingLeft;
    player->isSprinting = player->isRunning && runningFast;
    targetMaxSpeed = player->isSprinting ? 12 : 2;
    currentAcceleration = player->isSprinting ? 3 : 1;

    if (movingRight && !movingLeft) {
        if (player->speed < targetMaxSpeed) player->speed += currentAcceleration;
        if (player->speed > targetMaxSpeed) player->speed = targetMaxSpeed;
        player->direction = 0;
    } else if (movingLeft && !movingRight) {
        if (player->speed > -targetMaxSpeed) player->speed -= currentAcceleration;
        if (player->speed < -targetMaxSpeed) player->speed = -targetMaxSpeed;
        player->direction = 1;
    } else {
        if (player->speed > 0) player->speed--;
        else if (player->speed < 0) player->speed++;
    }

    if (player->isRunning || player->speed != 0) {
        player->frame = (player->frame + 1) % player->maxFrames;
        player->position.x += player->speed;
    }

    if (player->shootCooldown > 0) {
        player->shootCooldown--;
    }

    if (shooting && player->shootCooldown == 0) {
        int index = -1;
        for (int i = 0; i < MAX_SNOWBALLS; i++) {
            if (!snowballs[i].active) {
                index = i;
                break;
            }
        }
        if (index != -1) {
            Snowball *sb = &snowballs[index];
            sb->x = player->position.x + player->position.w;
            sb->y = player->position.y + (player->position.h / 2) - (sb->rect.h / 2);
            sb->speed = 14;
            sb->active = 1;
            sb->rect.x = sb->x;
            sb->rect.y = sb->y;
            sb->texture = snowballs[0].texture;
            if (index >= *snowballCount) {
                *snowballCount = index + 1;
            }
            player->shootCooldown = 12;
        }
    }
    
 
    if (player->position.x < 0) {
        player->position.x = 0;
    }
    

    if (player->position.x + player->position.w > SCREEN_WIDTH) {
        player->position.x = SCREEN_WIDTH - player->position.w;
    }

    if (!enemy->neutralized && player->position.x + player->position.w > enemy->position.x + 2) {
        player->position.x = enemy->position.x - player->position.w + 2;
        if (player->speed > 0) {
            player->speed = 0;
        }
    }
    
    for (int i = 0; i < *snowballCount; i++) {
        if (snowballs[i].active) {
            snowballs[i].x += snowballs[i].speed;
            snowballs[i].rect.x = snowballs[i].x;
            if (snowballs[i].x > SCREEN_WIDTH) {
                snowballs[i].active = 0;
            }
        }
    }

    applyScrolling(player, enemy, obs, snowballs, *snowballCount);
    player->position.x = PLAYER_FIXED_X;
}
/**
 * @brief Affiche le joueur, l'ennemi, l'obstacle et les boules de neige.
 * @param renderer Renderer SDL utilisé pour l'affichage.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Nombre de boules de neige.
 * @param player Pointeur vers le joueur.
 * @return Rien.
 */
void affichage(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Snowball snowballs[], int snowballCount, Player *player) {
    int frostLevel;
    int maxFrost;
    SDL_Texture *enemyTexture = NULL;
    if (enemy->displayDelay > 0) {
        enemy->displayDelay--;
    }
    if (obs->displayDelay > 0) {
        obs->displayDelay--;
    }
    maxFrost = 7;
    frostLevel = maxFrost - enemy->trees;
    if (frostLevel < 0) {
        frostLevel = 0;
    }
    if (frostLevel > maxFrost) {
        frostLevel = maxFrost;
    }
    
    if (enemy->animationState == 1 && enemy->textureWalk) {
        enemyTexture = enemy->textureWalk;
    } else if (enemy->animationState == 2 && enemy->textureRun) {
        enemyTexture = enemy->textureRun;
    } else {
        enemyTexture = enemy->textureNormal;
    }

    if (enemy->alive && enemyTexture && enemy->displayDelay <= 0) {
        if (enemy->playerTouch && enemy->playerTouchTimer < 60 && enemy->texturePlayerTouch) {
            int punchFrames[] = {6, 7, 8, 9, 10, 11, 12, 13, 14};
            int punchFrame = punchFrames[(enemy->playerTouchTimer / 3) % 9];
            SDL_Rect flashRect;
            updateEnemySpriteSize(enemy, enemy->texturePlayerTouch);
            enemy->posSprite.x = (punchFrame % enemy->nbCols) * enemy->frameWidth;
            enemy->posSprite.y = (punchFrame / enemy->nbCols) * enemy->frameHeight;
            flashRect = enemy->position;
            flashRect.x -= 8;
            flashRect.y -= 8;
            flashRect.w += 16;
            flashRect.h += 16;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            if ((enemy->playerTouchTimer / 4) % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 25, 25, 105);
                SDL_RenderFillRect(renderer, &flashRect);
                SDL_SetTextureColorMod(enemy->texturePlayerTouch, 255, 35, 35);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 230, 40, 95);
                SDL_RenderFillRect(renderer, &flashRect);
                SDL_SetTextureColorMod(enemy->texturePlayerTouch, 255, 220, 80);
            }
            SDL_SetTextureAlphaMod(enemy->texturePlayerTouch, 255);
            SDL_RenderCopy(renderer, enemy->texturePlayerTouch, &enemy->posSprite, &enemy->position);
        } else if (enemy->blinking) {
            if (enemy->textureHurt) {
                int hurtFrame = (enemy->blinkTimer / 2) % (enemy->nbCols * enemy->nbRows);
                updateEnemySpriteSize(enemy, enemy->textureHurt);
                enemy->posSprite.x = (hurtFrame % enemy->nbCols) * enemy->frameWidth;
                enemy->posSprite.y = (hurtFrame / enemy->nbCols) * enemy->frameHeight;
                if ((enemy->blinkTimer / 4) % 2 == 0) {
                    SDL_SetTextureColorMod(enemy->textureHurt, 255, 255, 255);
                } else {
                    SDL_SetTextureColorMod(enemy->textureHurt, 90, 210, 255);
                }
                SDL_SetTextureAlphaMod(enemy->textureHurt, 255);
                SDL_RenderCopy(renderer, enemy->textureHurt, &enemy->posSprite, &enemy->position);
            }
        } else if (enemy->hurt && enemy->textureHurt) {
            int hurtFrame = (enemy->hurtTimer / 2) % (enemy->nbCols * enemy->nbRows);
            updateEnemySpriteSize(enemy, enemy->textureHurt);
            enemy->posSprite.x = (hurtFrame % enemy->nbCols) * enemy->frameWidth;
            enemy->posSprite.y = (hurtFrame / enemy->nbCols) * enemy->frameHeight;
            SDL_SetTextureColorMod(enemy->textureHurt, 255, 255, 255);
            SDL_SetTextureAlphaMod(enemy->textureHurt, 255);
            SDL_RenderCopy(renderer, enemy->textureHurt, &enemy->posSprite, &enemy->position);
        } else {
            SDL_SetTextureColorMod(enemyTexture, enemy->skinTint.r, enemy->skinTint.g, enemy->skinTint.b);
            SDL_SetTextureAlphaMod(enemyTexture, 255);
            SDL_RenderCopy(renderer, enemyTexture, &enemy->posSprite, &enemy->position);
        }

        if (frostLevel > 0) {
            SDL_Rect frostBarBg = {enemy->position.x, enemy->position.y - 18, enemy->position.w, 8};
            SDL_Rect frostBar = {enemy->position.x, enemy->position.y - 18, (enemy->position.w * frostLevel) / maxFrost, 8};
            SDL_Rect frostLayer1 = {enemy->position.x + 8, enemy->position.y + enemy->position.h - 22, enemy->position.w - 16, 12};
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 40, 60, 90, 170);
            SDL_RenderFillRect(renderer, &frostBarBg);
            SDL_SetRenderDrawColor(renderer, 220, 240, 255, 220);
            SDL_RenderFillRect(renderer, &frostBar);
            SDL_SetRenderDrawColor(renderer, 235, 245, 255, 120);
            SDL_RenderFillRect(renderer, &frostLayer1);

            if (frostLevel >= 2) {
                SDL_Rect frostLayer2 = {enemy->position.x + 12, enemy->position.y + enemy->position.h - 40, enemy->position.w - 24, 10};
                SDL_RenderFillRect(renderer, &frostLayer2);
            }
            if (frostLevel >= 3) {
                SDL_Rect frostLayer3 = {enemy->position.x + 16, enemy->position.y + enemy->position.h - 58, enemy->position.w - 32, 10};
                SDL_RenderFillRect(renderer, &frostLayer3);
            }
            if (frostLevel >= 4) {
                SDL_Rect frostLayer4 = {enemy->position.x + 20, enemy->position.y + enemy->position.h - 76, enemy->position.w - 40, 10};
                SDL_RenderFillRect(renderer, &frostLayer4);
            }
            if (frostLevel >= 5) {
                SDL_Rect frostLayer5 = {enemy->position.x + 24, enemy->position.y + 12, enemy->position.w - 48, 12};
                SDL_RenderFillRect(renderer, &frostLayer5);
            }
            if (frostLevel >= 6) {
                SDL_Rect frostLayer6 = {enemy->position.x + 10, enemy->position.y + 6, 22, 22};
                SDL_Rect frostLayer7 = {enemy->position.x + enemy->position.w - 32, enemy->position.y + 6, 22, 22};
                SDL_RenderFillRect(renderer, &frostLayer6);
                SDL_RenderFillRect(renderer, &frostLayer7);
            }
            if (frostLevel >= 7) {
                SDL_Rect frostLayer8 = {enemy->position.x + 4, enemy->position.y + 4, enemy->position.w - 8, enemy->position.h - 8};
                SDL_SetRenderDrawColor(renderer, 245, 250, 255, 70);
                SDL_RenderFillRect(renderer, &frostLayer8);
            }
        }
    }
    if (obs->active && obs->texture && obs->displayDelay <= 0) {
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
/**
 * @brief Vérifie les collisions entre les éléments du jeu.
 * @param renderer Renderer SDL utilisé dans le jeu.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param collisionSound Son joué lors d'une collision.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Pointeur vers le nombre de boules de neige.
 * @param player Pointeur vers le joueur.
 * @return Rien.
 */
void checkCollision(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound, Snowball snowballs[], int *snowballCount, Player *player) {
    SDL_Rect playerTouchZone;
    int touchDistance;

    if (!enemy->alive) return;
    if (enemy->neutralized) return;
    if (obs->active && SDL_HasIntersection(&enemy->position, &obs->position)) {
        if (collisionSound) Mix_PlayChannel(-1, collisionSound, 0);
    }
    for (int i = 0; i < *snowballCount; i++) {
        if (snowballs[i].active && SDL_HasIntersection(&snowballs[i].rect, &enemy->position)) {
            enemy->blinking = 1;
            enemy->blinkTimer = 0;
            snowballs[i].active = 0;
            enemy->hurt = 1;
            enemy->hurtTimer = 0;
            if (enemy->trees > 0) enemy->trees--;
            if (enemy->trees <= 0) {
                enemy->neutralized = 1;
                enemy->trees = 0;
                enemy->currentSpeed = 0;
            }
        }
    }
    playerTouchZone = player->position;
    playerTouchZone.x += player->position.w - 12;
    playerTouchZone.w = 36;
    touchDistance = enemy->position.x - (player->position.x + player->position.w);

    if (enemy->touchCooldown == 0 && player->isSprinting && player->direction == 0 &&
        touchDistance <= 24 && touchDistance >= -24 &&
        SDL_HasIntersection(&playerTouchZone, &enemy->position)) {
        enemy->playerTouch = 1;
        enemy->playerTouchTimer = 0;
        enemy->touchCooldown = 90;
        enemy->startedMoving = 1;
        enemy->currentSpeed = 4;
        enemy->animationState = 2;

      
        enemy->position.x = player->position.x + player->position.w - 2;
    }
}
/**
 * @brief Gère les événements clavier du joueur.
 * @param event Événement SDL à traiter.
 * @param player Pointeur vers le joueur.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Pointeur vers le nombre de boules de neige.
 * @return Rien.
 */
void handleInput(SDL_Event *event, Player *player, Snowball snowballs[], int *snowballCount) {
    (void)player;
    (void)snowballs;
    (void)snowballCount;

    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
         
        }
    }
}
/**
 * @brief Gère le changement et l'affichage du background.
 * @param renderer Renderer SDL utilisé pour afficher le background.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param player Pointeur vers le joueur.
 * @return Rien.
 */
void scrollBackground(SDL_Renderer *renderer, Enemy *enemy, Obstacle *obs, Player *player) {
    static SDL_Texture *backgrounds[BACKGROUND_COUNT] = {0};
    int pageIndex;
    int nextPageIndex;
    int localOffset;
    SDL_Rect leftDst;
    SDL_Rect rightDst;

    (void)enemy;
    (void)obs;
    (void)player;

    for (int i = 0; i < BACKGROUND_COUNT; i++) {
        if (!backgrounds[i]) {
            backgrounds[i] = loadBackgroundTexture(renderer, i + 1);
        }
    }

    pageIndex = (gameScrollOffsetX / SCREEN_WIDTH) % BACKGROUND_COUNT;
    nextPageIndex = (pageIndex + 1) % BACKGROUND_COUNT;
    localOffset = gameScrollOffsetX % SCREEN_WIDTH;

    if (backgrounds[pageIndex]) {
        leftDst = (SDL_Rect){-localOffset, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, backgrounds[pageIndex], NULL, &leftDst);
    }

    if (backgrounds[nextPageIndex]) {
        rightDst = (SDL_Rect){SCREEN_WIDTH - localOffset, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_RenderCopy(renderer, backgrounds[nextPageIndex], NULL, &rightDst);
    }
}
/**
 * @brief Libère les textures et ressources utilisées par le jeu.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param collisionSound Son de collision à libérer.
 * @param snowball Pointeur vers la boule de neige.
 * @return Rien.
 */
void destroyAll(Enemy *enemy, Obstacle *obs, Mix_Chunk *collisionSound, Snowball *snowball) {
    if (enemy->textureNormal) SDL_DestroyTexture(enemy->textureNormal);
    if (enemy->textureHurt) SDL_DestroyTexture(enemy->textureHurt);
    if (enemy->texturePlayerTouch) SDL_DestroyTexture(enemy->texturePlayerTouch);
    if (enemy->textureWalk) SDL_DestroyTexture(enemy->textureWalk);
    if (enemy->textureRun) SDL_DestroyTexture(enemy->textureRun);
    if (enemy->treeTexture) SDL_DestroyTexture(enemy->treeTexture);
    if (obs->texture) SDL_DestroyTexture(obs->texture);
    if (snowball->texture) SDL_DestroyTexture(snowball->texture);
    if (collisionSound) Mix_FreeChunk(collisionSound);
    Mix_CloseAudio();
}
