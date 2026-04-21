#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <string.h>

#define WIDTH 1280
#define HEIGHT 720
#define MAX_PLAYERS 2
#define MAX_ENEMIES 10
#define MAX_PUZZLES 5
#define MAX_QUIZZES 3

typedef struct {
    Uint8 r, g, b, a;
} Color;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect position;
    int health;
    int score;
    int facing;
    int jumping;
} Player;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect position;
    int active;
    int direction;
    int speed;
} Enemy;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect position;
    int solved;
} Puzzle;

typedef struct {
    char question[256];
    char options[4][256];
    int correct_answer;
    int answered;
} Quiz;

typedef struct {
    SDL_Texture *texture;
    int scroll_x;
    int scroll_y;
} Background;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect rect;
    int x;
    int y;
} Minimap;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect rect;
    int selected;
} Button;

typedef struct {
    Player players[MAX_PLAYERS];
    Enemy enemies[MAX_ENEMIES];
    Puzzle puzzles[MAX_PUZZLES];
    Quiz quizzes[MAX_QUIZZES];
    Background background;
    Minimap minimap;
    Button buttons[6];
    int current_level;
    int game_running;
    int game_state;
} GameState;

void player_init(Player *p, SDL_Renderer *renderer, int x, int y);
void player_move(Player *p, int dx, int dy);
void player_jump(Player *p);
void player_render(SDL_Renderer *renderer, Player *p);

void enemy_init(Enemy *e, SDL_Renderer *renderer, int x, int y);
void enemy_update(Enemy *e);
void enemy_render(SDL_Renderer *renderer, Enemy *e);

void puzzle_init(Puzzle *pz, SDL_Renderer *renderer, int x, int y);
void puzzle_check(Puzzle *pz, Player *p);
void puzzle_render(SDL_Renderer *renderer, Puzzle *pz);

void quiz_init(Quiz *q, char *question, char *opt1, char *opt2, char *opt3, char *opt4, int correct);
void quiz_answer(Quiz *q, int choice);
void quiz_render(SDL_Renderer *renderer, Quiz *q, TTF_Font *font);

void background_init(Background *bg, SDL_Renderer *renderer, char *path);
void background_update(Background *bg);
void background_render(SDL_Renderer *renderer, Background *bg);

void minimap_init(Minimap *mm, SDL_Renderer *renderer);
void minimap_update(Minimap *mm, Player *p);
void minimap_render(SDL_Renderer *renderer, Minimap *mm);

void button_init(Button *btn, SDL_Renderer *renderer, int x, int y, int w, int h);
void button_render(SDL_Renderer *renderer, Button *btn);
int button_clicked(Button *btn, int mx, int my);

int game_init(GameState *gs, SDL_Renderer *renderer);
void game_update(GameState *gs);
void game_render(SDL_Renderer *renderer, GameState *gs);
void game_cleanup(GameState *gs);

#endif
