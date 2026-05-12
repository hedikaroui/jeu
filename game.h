/**
 * @file game.h
 * @brief Fichier contenant les structures et prototypes du jeu.
 * @author Maram Hnaien
 * @version 1
 * @date 2026
 */

#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stddef.h>

#define MAX_SNOWBALLS 20
#define MAX_SKINS 9
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

/**
 * @struct Skin
 * @brief Structure contenant les informations d'un skin de l'ennemi.
 */
typedef struct Skin {
    const char *name;            /*!< Nom du skin */
    const char *normalPath;      /*!< Chemin de l'image normale de l'ennemi */
    const char *hurtPath;        /*!< Chemin de l'image de l'ennemi blessé */
    const char *playerTouchPath; /*!< Chemin de l'image quand l'ennemi touche le joueur */
    const char *idlePath;        /*!< Chemin de l'image de l'ennemi en état immobile */
    const char *walkPath;        /*!< Chemin de l'image ou spritesheet de marche */
    const char *runPath;         /*!< Chemin de l'image ou spritesheet de course */
    SDL_Color tint;              /*!< Couleur appliquée au skin */
} Skin;

/**
 * @struct Enemy
 * @brief Structure représentant l'ennemi dans le jeu.
 */
typedef struct Enemy {
    SDL_Texture *textureNormal;      /*!< Texture normale de l'ennemi */
    SDL_Texture *textureHurt;        /*!< Texture de l'ennemi blessé */
    SDL_Texture *texturePlayerTouch; /*!< Texture utilisée quand l'ennemi touche le joueur */
    SDL_Texture *textureWalk;        /*!< Texture de marche de l'ennemi */
    SDL_Texture *textureRun;         /*!< Texture de course de l'ennemi */
    SDL_Texture *treeTexture;        /*!< Texture de l'arbre ou obstacle associé à l'ennemi */

    SDL_Rect position;               /*!< Position de l'ennemi sur l'écran */
    SDL_Rect posSprite;              /*!< Rectangle utilisé pour afficher une partie du spritesheet */

    int frameWidth;                  /*!< Largeur d'une frame du spritesheet */
    int frameHeight;                 /*!< Hauteur d'une frame du spritesheet */
    int frame;                       /*!< Frame actuelle de l'animation */
    int nbCols;                      /*!< Nombre de colonnes dans le spritesheet */
    int nbRows;                      /*!< Nombre de lignes dans le spritesheet */

    int direction;                   /*!< Direction de déplacement de l'ennemi */
    int health;                      /*!< Points de vie de l'ennemi */
    int alive;                       /*!< Indique si l'ennemi est vivant */
    int hurt;                        /*!< Indique si l'ennemi est blessé */
    int neutralized;                 /*!< Indique si l'ennemi est neutralisé */
    int playerTouch;                 /*!< Indique si l'ennemi touche le joueur */

    int hurtTimer;                   /*!< Compteur de durée de l'état blessé */
    int playerTouchTimer;            /*!< Compteur de durée du contact avec le joueur */
    int touchCooldown;               /*!< Temps d'attente avant un nouveau contact */
    int recoilTimer;                 /*!< Compteur du recul de l'ennemi */
    int recoiling;                   /*!< Indique si l'ennemi est en recul */
    int blinkTimer;                  /*!< Compteur de clignotement */
    int blinking;                    /*!< Indique si l'ennemi clignote */

    int trees;                       /*!< Nombre d'arbres ou obstacles liés à l'ennemi */
    int currentSpeed;                /*!< Vitesse actuelle de l'ennemi */
    int displayDelay;                /*!< Délai avant l'affichage ou l'action */
    int startedMoving;               /*!< Indique si l'ennemi a commencé à bouger */
    int animationState;              /*!< État actuel de l'animation */

    SDL_Color skinTint;              /*!< Couleur du skin appliquée à l'ennemi */
    const char *skinName;            /*!< Nom du skin choisi */
} Enemy;

/**
 * @struct Obstacle
 * @brief Structure représentant un obstacle dans le jeu.
 */
typedef struct Obstacle {
    SDL_Texture *texture; /*!< Texture de l'obstacle */
    SDL_Rect position;    /*!< Position de l'obstacle sur l'écran */
    int active;           /*!< Indique si l'obstacle est actif */
    int displayDelay;     /*!< Délai avant l'affichage de l'obstacle */
} Obstacle;

/**
 * @struct Snowball
 * @brief Structure représentant une boule de neige lancée par le joueur.
 */
typedef struct Snowball {
    SDL_Texture *texture; /*!< Texture de la boule de neige */
    SDL_Rect rect;        /*!< Rectangle de position et de taille de la boule de neige */
    int x;                /*!< Position horizontale de la boule de neige */
    int y;                /*!< Position verticale de la boule de neige */
    int speed;            /*!< Vitesse de déplacement de la boule de neige */
    int active;           /*!< Indique si la boule de neige est active */
} Snowball;

/**
 * @struct Player
 * @brief Structure représentant le joueur dans le jeu.
 */
typedef struct Player {
    SDL_Texture *texture; /*!< Texture ou spritesheet du joueur */
    SDL_Rect position;    /*!< Position du joueur sur l'écran */

    int frame;            /*!< Frame actuelle de l'animation du joueur */
    int maxFrames;        /*!< Nombre maximal de frames de l'animation */
    int frameWidth;       /*!< Largeur d'une frame du joueur */
    int frameHeight;      /*!< Hauteur d'une frame du joueur */

    int speed;            /*!< Vitesse actuelle du joueur */
    int isRunning;        /*!< Indique si le joueur court */
    int isSprinting;      /*!< Indique si le joueur sprinte */
    int direction;        /*!< Direction du joueur */
    int acceleration;     /*!< Accélération du joueur */
    int maxSpeed;         /*!< Vitesse maximale du joueur */
    int shootCooldown;    /*!< Temps d'attente entre deux tirs */
} Player;

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
 * @param playerTouchPath Chemin de l'image utilisée lors du contact avec le joueur.
 * @param walkPath Chemin du spritesheet de marche.
 * @param runPath Chemin du spritesheet de course.
 * @param treePath Chemin de l'image de l'arbre.
 * @param obs Pointeur vers l'obstacle à initialiser.
 * @param obsPath Chemin de l'image de l'obstacle.
 * @param x Position horizontale de l'obstacle.
 * @param y Position verticale de l'obstacle.
 * @param sb Pointeur vers la boule de neige.
 * @param snowPath Chemin de l'image de la boule de neige.
 * @param player Pointeur vers le joueur à initialiser.
 * @param playerPath Chemin du spritesheet du joueur.
 * @param nbCols Nombre de colonnes du spritesheet du joueur.
 * @param nbRows Nombre de lignes du spritesheet du joueur.
 * @return Rien.
 */
void init(SDL_Renderer *renderer,
          Enemy    *enemy,  char *normalPath, int normalCols, int normalRows,
                            char *hurtPath,   int hurtCols,   int hurtRows,
                            char *playerTouchPath,
                            char *walkPath,
                            char *runPath,
                            char *treePath,
          Obstacle *obs,    char *obsPath,    int x,          int y,
          Snowball *sb,     char *snowPath,
          Player   *player, char *playerPath, int nbCols,     int nbRows);

/**
 * @brief Affiche les éléments du jeu sur l'écran.
 * @param renderer Renderer SDL utilisé pour l'affichage.
 * @param enemy Pointeur vers l'ennemi à afficher.
 * @param obs Pointeur vers l'obstacle à afficher.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Nombre de boules de neige actives.
 * @param player Pointeur vers le joueur à afficher.
 * @return Rien.
 */
void affichage(SDL_Renderer *renderer,
               Enemy *enemy, Obstacle *obs,
               Snowball snowballs[], int snowballCount,
               Player *player);

/**
 * @brief Déplace l'ennemi dans le jeu.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param player Pointeur vers le joueur.
 * @return Rien.
 */
void move(Enemy *enemy, Obstacle *obs, Player *player);

/**
 * @brief Met à jour l'état et le mouvement du joueur.
 * @param player Pointeur vers le joueur.
 * @param enemy Pointeur vers l'ennemi.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Pointeur vers le nombre de boules de neige.
 * @return Rien.
 */
void updatePlayer(Player *player, Enemy *enemy, Obstacle *obs,
                  Snowball snowballs[], int *snowballCount);

/**
 * @brief Vérifie les collisions entre le joueur, l'ennemi, l'obstacle et les boules de neige.
 * @param renderer Renderer SDL utilisé dans le jeu.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param collisionSound Son joué lors d'une collision.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Pointeur vers le nombre de boules de neige.
 * @param player Pointeur vers le joueur.
 * @return Rien.
 */
void checkCollision(SDL_Renderer *renderer,
                    Enemy *enemy, Obstacle *obs,
                    Mix_Chunk *collisionSound,
                    Snowball snowballs[], int *snowballCount,
                    Player *player);

/**
 * @brief Gère les entrées clavier ou événements SDL du joueur.
 * @param event Événement SDL à traiter.
 * @param player Pointeur vers le joueur.
 * @param snowballs Tableau des boules de neige.
 * @param snowballCount Pointeur vers le nombre de boules de neige.
 * @return Rien.
 */
void handleInput(SDL_Event *event, Player *player,
                 Snowball snowballs[], int *snowballCount);

/**
 * @brief Fait défiler le background et met à jour les positions des objets.
 * @param renderer Renderer SDL utilisé pour l'affichage.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param player Pointeur vers le joueur.
 * @return Rien.
 */
void scrollBackground(SDL_Renderer *renderer, Enemy *enemy,
                      Obstacle *obs, Player *player);

/**
 * @brief Affiche l'écran de sélection des skins.
 * @param renderer Renderer SDL utilisé pour l'affichage.
 * @param font Police utilisée pour afficher le texte.
 * @param skins Tableau contenant les skins disponibles.
 * @param skinCount Nombre de skins disponibles.
 * @return L'indice du skin choisi par le joueur.
 */
int showSkinSelectionScreen(SDL_Renderer *renderer, TTF_Font *font,
                            const Skin skins[], int skinCount);

/**
 * @brief Applique un skin à l'ennemi.
 * @param enemy Pointeur vers l'ennemi.
 * @param skin Pointeur vers le skin choisi.
 * @return Rien.
 */
void setEnemySkin(Enemy *enemy, const Skin *skin);

/**
 * @brief Résout le chemin complet d'un fichier asset.
 * @param baseName Nom de base du fichier.
 * @param output Chaîne de caractères où le chemin final sera stocké.
 * @param outputSize Taille maximale de la chaîne output.
 * @return 1 si le chemin est trouvé, 0 sinon.
 */
int resolveAssetPath(const char *baseName, char *output, size_t outputSize);

/**
 * @brief Charge une texture SDL à partir d'un chemin.
 * @param renderer Renderer SDL utilisé pour créer la texture.
 * @param path Chemin de l'image à charger.
 * @return La texture chargée, ou NULL en cas d'erreur.
 */
SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *path);

/**
 * @brief Dessine un rectangle rempli.
 * @param renderer Renderer SDL utilisé pour dessiner.
 * @param rect Rectangle à dessiner.
 * @param color Couleur du rectangle.
 * @return Rien.
 */
void drawFilledRect(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color);

/**
 * @brief Dessine le contour d'un rectangle.
 * @param renderer Renderer SDL utilisé pour dessiner.
 * @param rect Rectangle à dessiner.
 * @param color Couleur du contour.
 * @return Rien.
 */
void drawRectOutline(SDL_Renderer *renderer, SDL_Rect rect, SDL_Color color);

/**
 * @brief Crée une texture contenant du texte.
 * @param renderer Renderer SDL utilisé pour créer la texture.
 * @param font Police utilisée pour écrire le texte.
 * @param text Texte à transformer en texture.
 * @param color Couleur du texte.
 * @param w Pointeur où sera stockée la largeur du texte.
 * @param h Pointeur où sera stockée la hauteur du texte.
 * @return La texture du texte créée, ou NULL en cas d'erreur.
 */
SDL_Texture *createTextTexture(SDL_Renderer *renderer, TTF_Font *font,
                               const char *text, SDL_Color color,
                               int *w, int *h);

/**
 * @brief Libère les ressources utilisées par le jeu.
 * @param enemy Pointeur vers l'ennemi.
 * @param obs Pointeur vers l'obstacle.
 * @param collisionSound Son de collision à libérer.
 * @param snowball Pointeur vers la boule de neige.
 * @return Rien.
 */
void destroyAll(Enemy *enemy, Obstacle *obs,
                Mix_Chunk *collisionSound, Snowball *snowball);

#endif
