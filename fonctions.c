#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "header.h"

#define MAX_SAVED_GAMES 100
#define SAVE_START "=== SAVE START ==="
#define SAVE_END   "=== SAVE END ==="

static void trim_newline(char *text)
{
    size_t len = strlen(text);

    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        len--;
    }
}

static int read_string_value(const char *line, const char *prefix, char *dest, size_t dest_size)
{
    size_t prefix_len = strlen(prefix);

    if (strncmp(line, prefix, prefix_len) != 0) {
        return 0;
    }

    snprintf(dest, dest_size, "%s", line + prefix_len);
    trim_newline(dest);
    return 1;
}

static int read_int_value(const char *line, const char *prefix, int *value)
{
    size_t prefix_len = strlen(prefix);

    if (strncmp(line, prefix, prefix_len) != 0) {
        return 0;
    }

    return sscanf(line + prefix_len, "%d", value) == 1;
}

static void generate_game_id(GameData *data)
{
    static int seeded = 0;

    if (data->game_id[0] != '\0') {
        return;
    }

    if (!seeded) {
        srand((unsigned int) time(NULL));
        seeded = 1;
    }

    snprintf(data->game_id, sizeof(data->game_id), "GAME-%ld-%u",
             (long) time(NULL), (unsigned int) (rand() % 100000));
}

static void write_save_block(FILE *file, const GameData *data)
{
    fprintf(file, "%s\n", SAVE_START);
    fprintf(file, "GAME_ID: %s\n", data->game_id);
    fprintf(file, "MODE: %s\n", data->mode);
    fprintf(file, "CURRENT_LEVEL: %d\n", data->current_level);
    fprintf(file, "INPUT_DEVICE: %s\n", data->input_device);
    fprintf(file, "PLAYER1_NAME: %s\n", data->player1_name);
    fprintf(file, "PLAYER1_CHARACTER: %s\n", data->player1_character);
    fprintf(file, "PLAYER1_POSITION_X: %d\n", data->player1_x);
    fprintf(file, "PLAYER1_POSITION_Y: %d\n", data->player1_y);
    fprintf(file, "PLAYER1_LIVES: %d\n", data->player1_lives);
    fprintf(file, "PLAYER1_SCORE: %d\n", data->player1_score);

    if (strcmp(data->mode, "multi") == 0) {
        fprintf(file, "PLAYER2_NAME: %s\n", data->player2_name);
        fprintf(file, "PLAYER2_CHARACTER: %s\n", data->player2_character);
        fprintf(file, "PLAYER2_POSITION_X: %d\n", data->player2_x);
        fprintf(file, "PLAYER2_POSITION_Y: %d\n", data->player2_y);
        fprintf(file, "PLAYER2_LIVES: %d\n", data->player2_lives);
        fprintf(file, "PLAYER2_SCORE: %d\n", data->player2_score);
    } else {
        fprintf(file, "PLAYER2_NAME: N/A\n");
        fprintf(file, "PLAYER2_CHARACTER: N/A\n");
        fprintf(file, "PLAYER2_POSITION_X: 0\n");
        fprintf(file, "PLAYER2_POSITION_Y: 0\n");
        fprintf(file, "PLAYER2_LIVES: 0\n");
        fprintf(file, "PLAYER2_SCORE: 0\n");
    }

    fprintf(file, "%s\n", SAVE_END);
}

static int read_save_block(FILE *file, GameData *data)
{
    char line[256];

    memset(data, 0, sizeof(*data));

    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);
        if (strcmp(line, SAVE_START) == 0) {
            break;
        }
    }

    if (feof(file)) {
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);

        if (strcmp(line, SAVE_END) == 0) {
            return data->game_id[0] != '\0';
        }

        if (read_string_value(line, "GAME_ID: ", data->game_id, sizeof(data->game_id))) {
            continue;
        }
        if (read_string_value(line, "MODE: ", data->mode, sizeof(data->mode))) {
            continue;
        }
        if (read_int_value(line, "CURRENT_LEVEL: ", &data->current_level)) {
            continue;
        }
        if (read_string_value(line, "INPUT_DEVICE: ", data->input_device, sizeof(data->input_device))) {
            continue;
        }
        if (read_string_value(line, "PLAYER1_NAME: ", data->player1_name, sizeof(data->player1_name))) {
            continue;
        }
        if (read_string_value(line, "PLAYER1_CHARACTER: ", data->player1_character, sizeof(data->player1_character))) {
            continue;
        }
        if (read_int_value(line, "PLAYER1_POSITION_X: ", &data->player1_x)) {
            continue;
        }
        if (read_int_value(line, "PLAYER1_POSITION_Y: ", &data->player1_y)) {
            continue;
        }
        if (read_int_value(line, "PLAYER1_LIVES: ", &data->player1_lives)) {
            continue;
        }
        if (read_int_value(line, "PLAYER1_SCORE: ", &data->player1_score)) {
            continue;
        }
        if (read_string_value(line, "PLAYER2_NAME: ", data->player2_name, sizeof(data->player2_name))) {
            continue;
        }
        if (read_string_value(line, "PLAYER2_CHARACTER: ", data->player2_character, sizeof(data->player2_character))) {
            continue;
        }
        if (read_int_value(line, "PLAYER2_POSITION_X: ", &data->player2_x)) {
            continue;
        }
        if (read_int_value(line, "PLAYER2_POSITION_Y: ", &data->player2_y)) {
            continue;
        }
        if (read_int_value(line, "PLAYER2_LIVES: ", &data->player2_lives)) {
            continue;
        }
        if (read_int_value(line, "PLAYER2_SCORE: ", &data->player2_score)) {
            continue;
        }
    }

    return 0;
}

int InitSDL(SaveGame *s)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        printf("Erreur IMG_Init : %s\n", IMG_GetError());
        return 0;
    }
    if (TTF_Init() == -1) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        return 0;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        printf("Erreur Mix_OpenAudio : %s\n", Mix_GetError());
        return 0;
    }
    return 1;
}

int Initialisation(SaveGame *s)
{
    s->background   = NULL;
    s->font         = NULL;
    s->titleSurface = NULL;
    s->titleTexture = NULL;
    s->music        = NULL;
    s->sound        = NULL;
    s->clic_bouton  = -1;

    for (int i = 0; i < 4; i++) {
        s->buttons[i].texture     = NULL;
        s->buttons[i].textureCliq = NULL;
        s->buttons[i].selected    = 0;
    }

    s->game_data.game_id[0] = '\0';
    strcpy(s->game_data.mode, "mono");
    s->game_data.current_level = 1;
    strcpy(s->game_data.input_device, "keyboard");

    strcpy(s->game_data.player1_name, "Player1");
    strcpy(s->game_data.player1_character, "Unknown");
    s->game_data.player1_x = 100;
    s->game_data.player1_y = 100;
    s->game_data.player1_lives = 3;
    s->game_data.player1_score = 0;

    strcpy(s->game_data.player2_name, "N/A");
    strcpy(s->game_data.player2_character, "N/A");
    s->game_data.player2_x = 0;
    s->game_data.player2_y = 0;
    s->game_data.player2_lives = 0;
    s->game_data.player2_score = 0;

    s->running = 1;
    s->etat    = 0;

    return 1;
}

int Load(SaveGame *s)
{
    s->background = IMG_LoadTexture(s->renderer, "BG.png");
    if (!s->background) {
        printf("Erreur chargement BG.png : %s\n", IMG_GetError());
        return 0;
    }

    s->music = Mix_LoadMUS("music.mp3");
    if (!s->music) {
        printf("Erreur chargement music.mp3 : %s\n", Mix_GetError());
        return 0;
    }
    Mix_PlayMusic(s->music, -1);

    s->sound = Mix_LoadWAV("cliq.wav");
    if (!s->sound) {
        printf("Erreur chargement cliq.wav : %s\n", Mix_GetError());
        return 0;
    }

    s->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    if (!s->font) {
        printf("Erreur chargement font : %s\n", TTF_GetError());
        return 0;
    }

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red   = {178, 34, 34, 255};
    s->titleSurface = TTF_RenderText_Shaded(s->font, "Do you want to save the game", white, red);
    if (!s->titleSurface) {
        printf("Erreur titleSurface : %s\n", TTF_GetError());
        return 0;
    }
    s->titleTexture = SDL_CreateTextureFromSurface(s->renderer, s->titleSurface);
    s->titleRect    = (SDL_Rect){ (WIDTH - s->titleSurface->w) / 2, 20,
                                   s->titleSurface->w, s->titleSurface->h };

    s->buttons[0].rect        = (SDL_Rect){150, 350, 300, 300};
    s->buttons[0].texture     = IMG_LoadTexture(s->renderer, "yesno.png");
    s->buttons[0].textureCliq = IMG_LoadTexture(s->renderer, "yesyes.png");

    s->buttons[1].rect        = (SDL_Rect){500, 350, 300, 300};
    s->buttons[1].texture     = IMG_LoadTexture(s->renderer, "nono.png");
    s->buttons[1].textureCliq = IMG_LoadTexture(s->renderer, "noyes.png");

    s->buttons[2].rect        = (SDL_Rect){200, 200, 400, 150};
    s->buttons[2].texture     = IMG_LoadTexture(s->renderer, "Sauvnon.png");
    s->buttons[2].textureCliq = IMG_LoadTexture(s->renderer, "Sauvyes.png");

    s->buttons[3].rect        = (SDL_Rect){200, 370, 400, 150};
    s->buttons[3].texture     = IMG_LoadTexture(s->renderer, "Newgameno.png");
    s->buttons[3].textureCliq = IMG_LoadTexture(s->renderer, "Nouvpartieyes.png");

    for (int i = 0; i < 4; i++) {
        if (!s->buttons[i].texture || !s->buttons[i].textureCliq) {
            printf("Erreur chargement texture bouton %d : %s\n", i, IMG_GetError());
            return 0;
        }
    }

    return 1;
}

void Affichage(SaveGame *s)
{
    SDL_SetRenderDrawColor(s->renderer, 255, 255, 255, 255);
    SDL_RenderClear(s->renderer);
    SDL_RenderCopy(s->renderer, s->background, NULL, NULL);

    if (s->etat == 0) {
        SDL_RenderCopy(s->renderer, s->titleTexture, NULL, &s->titleRect);

        if (s->buttons[0].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[0].textureCliq, NULL, &s->buttons[0].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[0].texture, NULL, &s->buttons[0].rect);

        if (s->buttons[1].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[1].textureCliq, NULL, &s->buttons[1].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[1].texture, NULL, &s->buttons[1].rect);

    } else {

        if (s->buttons[2].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[2].textureCliq, NULL, &s->buttons[2].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[2].texture, NULL, &s->buttons[2].rect);

        if (s->buttons[3].selected == 1)
            SDL_RenderCopy(s->renderer, s->buttons[3].textureCliq, NULL, &s->buttons[3].rect);
        else
            SDL_RenderCopy(s->renderer, s->buttons[3].texture, NULL, &s->buttons[3].rect);
    }

    SDL_RenderPresent(s->renderer);
}

void LectureEntree(SaveGame *s)
{
    SDL_Event event;
    s->clic_bouton = -1;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT)
            s->running = 0;

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                s->running = 0;
            if (event.key.keysym.sym == SDLK_n)
                s->clic_bouton = 3;
        }

        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x, my = event.motion.y;
            for (int i = 0; i < 4; i++) s->buttons[i].selected = 0;

            if (s->etat == 0) {
                if (mx >= s->buttons[0].rect.x && mx <= s->buttons[0].rect.x + s->buttons[0].rect.w &&
                    my >= s->buttons[0].rect.y && my <= s->buttons[0].rect.y + s->buttons[0].rect.h)
                    s->buttons[0].selected = 1;

                if (mx >= s->buttons[1].rect.x && mx <= s->buttons[1].rect.x + s->buttons[1].rect.w &&
                    my >= s->buttons[1].rect.y && my <= s->buttons[1].rect.y + s->buttons[1].rect.h)
                    s->buttons[1].selected = 1;
            } else {
                if (mx >= s->buttons[2].rect.x && mx <= s->buttons[2].rect.x + s->buttons[2].rect.w &&
                    my >= s->buttons[2].rect.y && my <= s->buttons[2].rect.y + s->buttons[2].rect.h)
                    s->buttons[2].selected = 1;

                if (mx >= s->buttons[3].rect.x && mx <= s->buttons[3].rect.x + s->buttons[3].rect.w &&
                    my >= s->buttons[3].rect.y && my <= s->buttons[3].rect.y + s->buttons[3].rect.h)
                    s->buttons[3].selected = 1;
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;

            if (s->etat == 0) {
                if (mx >= s->buttons[0].rect.x && mx <= s->buttons[0].rect.x + s->buttons[0].rect.w &&
                    my >= s->buttons[0].rect.y && my <= s->buttons[0].rect.y + s->buttons[0].rect.h)
                    s->clic_bouton = 0;

                if (mx >= s->buttons[1].rect.x && mx <= s->buttons[1].rect.x + s->buttons[1].rect.w &&
                    my >= s->buttons[1].rect.y && my <= s->buttons[1].rect.y + s->buttons[1].rect.h)
                    s->clic_bouton = 1;
            } else {
                if (mx >= s->buttons[2].rect.x && mx <= s->buttons[2].rect.x + s->buttons[2].rect.w &&
                    my >= s->buttons[2].rect.y && my <= s->buttons[2].rect.y + s->buttons[2].rect.h)
                    s->clic_bouton = 2;

                if (mx >= s->buttons[3].rect.x && mx <= s->buttons[3].rect.x + s->buttons[3].rect.w &&
                    my >= s->buttons[3].rect.y && my <= s->buttons[3].rect.y + s->buttons[3].rect.h)
                    s->clic_bouton = 3;
            }
        }
    }
}

void MiseAJour(SaveGame *s)
{
    if (s->clic_bouton == 0) {
        Mix_PlayChannel(-1, s->sound, 0);
        if (save_game(&s->game_data)) {
            printf("Partie sauvegardee avec succes\n");
        } else {
            printf("Erreur lors de la sauvegarde\n");
        }
        s->etat = 1;
    }
    if (s->clic_bouton == 1) {
        Mix_PlayChannel(-1, s->sound, 0);
        printf("You chose no \n");
    }
    if (s->clic_bouton == 2) {
        Mix_PlayChannel(-1, s->sound, 0);
        printf("Save done\n");
    }
    if (s->clic_bouton == 3) {
        Mix_PlayChannel(-1, s->sound, 0);
        printf("New game loaded\n");
    }

    SDL_Delay(16);
}

void Liberation(SaveGame *s)
{
    SDL_FreeSurface(s->titleSurface);
    SDL_DestroyTexture(s->titleTexture);
    SDL_DestroyTexture(s->background);

    for (int i = 0; i < 4; i++) {
        SDL_DestroyTexture(s->buttons[i].texture);
        SDL_DestroyTexture(s->buttons[i].textureCliq);
    }

    TTF_CloseFont(s->font);
    Mix_FreeMusic(s->music);
    Mix_FreeChunk(s->sound);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();

    SDL_DestroyRenderer(s->renderer);
    SDL_DestroyWindow(s->window);
    SDL_Quit();
}

int save_game(GameData *data)
{
    FILE *file = NULL;
    GameData saved_games[MAX_SAVED_GAMES];
    int game_count = 0;
    int found = 0;
    int i;

    generate_game_id(data);

    file = fopen("savegame.txt", "r");
    if (file) {
        while (game_count < MAX_SAVED_GAMES && read_save_block(file, &saved_games[game_count])) {
            if (strcmp(saved_games[game_count].game_id, data->game_id) == 0) {
                saved_games[game_count] = *data;
                found = 1;
            }
            game_count++;
        }
        fclose(file);
    }

    if (!found) {
        if (game_count >= MAX_SAVED_GAMES) {
            printf("Erreur: nombre maximum de sauvegardes atteint\n");
            return 0;
        }
        saved_games[game_count] = *data;
        game_count++;
    }

    file = fopen("savegame.txt", "w");
    if (!file) {
        printf("Erreur: impossible d'ouvrir le fichier de sauvegarde\n");
        return 0;
    }

    for (i = 0; i < game_count; i++) {
        write_save_block(file, &saved_games[i]);
    }

    fclose(file);
    return 1;
}

int load_game(GameData *data)
{
    FILE *file = fopen("savegame.txt", "r");
    GameData saved_game;

    if (!file) {
        printf("Erreur: fichier de sauvegarde introuvable\n");
        return 0;
    }

    while (read_save_block(file, &saved_game)) {
        if (data->game_id[0] == '\0' || strcmp(saved_game.game_id, data->game_id) == 0) {
            *data = saved_game;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    printf("Erreur: sauvegarde introuvable\n");
    return 0;
}
