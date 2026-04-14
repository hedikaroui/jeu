#include "bg.h"
#include "minimap.h"
#include "enemi.h"
#include "enigme22.h"
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Game g;
    memset(&g, 0, sizeof(Game));
    g.state = STATE_MENU;
    srand((unsigned int)time(NULL));

    if (!InitSDL(&g, "Mon Jeu SDL2", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;

    LoadBackground(&g, "background lvl 1.jpg");
    LoadNameBackground(&g, "saisir.png");
    InitBackground(&g, SCREEN_WIDTH, SCREEN_HEIGHT);
    InitPlatforms(&g, 1);
    InitPlayer(&g.player1, g.renderer, NULL, 100, 300);
    InitPlayer(&g.player2, g.renderer, NULL, 1000, 300);

    Minimap mm;
    GameState mmState;
    Entite dummyEntite = {0};
    if (!LoadRessources(&mm, &mmState, g.renderer,
                        "assets/backgound.bmp", "assets/mario.png",
                        SCREEN_WIDTH - 220, 10, 200, 130,
                        16, 16, 20,
                        (SDL_Rect){ 100, 300, 50, 60 }, 0.0f, 1.0f)) {
        printf("Erreur LoadRessources minimap\n");
        return 1;
    }

    SDL_Surface *maskSurf = SDL_LoadBMP("assets/backgoundmasq.bmp");
    if (!maskSurf) fprintf(stderr, "[PP] Masque introuvable\n");

    Etincelle etincelle;
    LoadEtincelle(&etincelle, g.renderer, "assets/anim.png", 5, 2);

    /* Enigme popup state */
    enigme22 enigme;
    int enigmeOpen = 0;
    Uint32 nextEnigmeTick = 0;


    Enemy enemy;
    Obstacle spider;
    Obstacle falling;
    HUD hud;
    Snowball snowballs[MAX_SNOWBALLS];
    int snowballCount = 0;
    EPlayer player;
    Mix_Chunk *collisionSound = NULL;
    Mix_Music *backgroundMusic = NULL;

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    backgroundMusic = Mix_LoadMUS("jingle.mp3");
    if (backgroundMusic) {
        Mix_PlayMusic(backgroundMusic, -1);
    } else {
        fprintf(stderr, "Mix_LoadMUS jingle.mp3 failed: %s\n", Mix_GetError());
    }
    collisionSound = Mix_LoadWAV("sound.wav");
    if (!collisionSound) {
        fprintf(stderr, "Mix_LoadWAV sound.wav failed: %s\n", Mix_GetError());
    }

    initEnemy(g.renderer, &enemy, "enemy.png", 5, 5, "enemy2.png", 5, 2);
    initObstacle(g.renderer, &spider, "spider.png", 600, 300);
    initObstacle(g.renderer, &falling, "falling.png", 400, 200);
    initHUD(&hud, g.renderer, "tree.png");
    initSnowball(g.renderer, &snowballs[0], "snowball.png");
    for (int i = 1; i < MAX_SNOWBALLS; i++) {
        snowballs[i].texture = snowballs[0].texture;
        snowballs[i].rect.w = snowballs[0].rect.w;
        snowballs[i].rect.h = snowballs[0].rect.h;
        snowballs[i].active = 0;
    }
    initPlayer(g.renderer, &player, "player.png", 5, 5, &enemy);

    SDL_StartTextInput();
    int running = 1, keys[SDL_NUM_SCANCODES] = {0};
    SDL_Event e;

    
    while (running) {

        if (g.state == STATE_MENU) {
            RenderMenu(&g);
        } else if (g.state == STATE_NAME_INPUT) {
            RenderNameInput(&g);
        } else if (g.state == STATE_GAME) {

            if (g.splitScreen)
                RenderGameSplit(&g, keys);
            else
                RenderGameSingle(&g, keys);

            afficherMinimap(&mm, g.renderer, &mmState, &dummyEntite);
            afficherEtincelle(g.renderer, &etincelle);
            renderBorder(g.renderer, mm.minimapPosition, mmState.borderTimer);
            if (g.splitScreen) {
                SDL_Rect vp1 = g.background.posEcran1;
                SDL_Rect cam1 = g.background.camera1;
                SDL_RenderSetViewport(g.renderer, &vp1);
                renderEnemy(g.renderer, &enemy, &cam1);
                SDL_RenderSetViewport(g.renderer, NULL);
            } else {
                renderEnemy(g.renderer, &enemy, &g.background.camera1);
            }
            renderObstacle(g.renderer, &spider);
            renderObstacle(g.renderer, &falling);
            renderSnowballs(g.renderer, snowballs, snowballCount);
            renderHUD(g.renderer, &hud);
            renderPlayer(g.renderer, &player);
            if (enigmeOpen) {
                E22_Affichage(&enigme, g.renderer);
                E22_MiseAJour(&enigme);
            }
            SDL_RenderPresent(g.renderer);
        }


        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { running = 0; break; }
            if (g.state == STATE_MENU) {
                if (e.type == SDL_MOUSEBUTTONDOWN)
                    HandleMenuClick(&g, e.button.x, e.button.y);
            } else if (g.state == STATE_NAME_INPUT) {
                HandleNameInput(&g, &e);
            } else if (g.state == STATE_GAME) {
                /* If enigme popup is open, forward events to it and skip normal game input */
                if (enigmeOpen) {
                    int fermer = 0;
                    E22_GererEvenement(&enigme, &e, &fermer);
                    if (fermer) { E22_Liberation(&enigme); enigmeOpen = 0; }
                } else {
                    if (e.type == SDL_MOUSEBUTTONDOWN)
                        HandleGuideClick(&g, e.button.x, e.button.y);

                    handlePlayerMovement(&e, &player);
                    handleSnowballThrow(&e, &player, snowballs, &snowballCount);

                    if (e.type == SDL_KEYDOWN) {
                        SDL_Scancode sc = e.key.keysym.scancode;
                        if (sc == SDL_SCANCODE_H || sc == SDL_SCANCODE_F1)
                            g.showGuide = !g.showGuide;
                        else if (sc == SDL_SCANCODE_ESCAPE) {
                            if (g.showGuide) g.showGuide = 0;
                            else { memset(keys, 0, sizeof(keys)); g.state = STATE_MENU; }
                        } else
                            keys[sc] = 1;
                        if (g.splitScreen) printf("DEBUG KEYDOWN sc=%d(%s) keys[%d]=%d\n", sc, SDL_GetScancodeName(sc), sc, keys[sc]);
                        if (sc == SDL_SCANCODE_B) {
                            mmState.zoom += ZOOM_STEP;
                            if (mmState.zoom > ZOOM_MAX) mmState.zoom = ZOOM_MAX;
                        }
                        if (sc == SDL_SCANCODE_N) {
                            mmState.zoom -= ZOOM_STEP;
                            if (mmState.zoom < ZOOM_MIN) mmState.zoom = ZOOM_MIN;
                        }
                    }
                    if (e.type == SDL_KEYUP) {
                        keys[e.key.keysym.scancode] = 0;
                        if (g.splitScreen) printf("DEBUG KEYUP sc=%d(%s) keys[%d]=%d\n", e.key.keysym.scancode, SDL_GetScancodeName(e.key.keysym.scancode), e.key.keysym.scancode, keys[e.key.keysym.scancode]);
                    }
                }
            }
        }


        if (g.state == STATE_GAME) {
            Uint32 now = SDL_GetTicks();
            if (nextEnigmeTick == 0)
                nextEnigmeTick = now + 5000 + (Uint32)(rand() % 10001);

            if (!enigmeOpen && now >= nextEnigmeTick) {
                if (!E22_Initialisation(&enigme) || !E22_Load(&enigme, g.renderer)) {
                    fprintf(stderr, "[E22] impossible de charger l'enigme\n");
                    nextEnigmeTick = now + 8000;
                } else {
                    enigmeOpen = 1;
                    nextEnigmeTick = now + 15000 + (Uint32)(rand() % 15001);
                }
            }

            if (enigmeOpen) {
                SDL_Delay(16);
                continue;
            }

            SDL_Rect oldPos = g.player1.rect;
            if (g.splitScreen)
                UpdatePlayersSplit(&g, keys, LEVEL_WIDTH, LEVEL_HEIGHT);
            else
                UpdatePlayersSingle(&g, keys, LEVEL_WIDTH, LEVEL_HEIGHT);


            moveAndAnimateEnemy(&enemy);
            if (g.splitScreen) {
                int camX = g.background.camera1.x;
                int camW = g.background.camera1.w;
                if (enemy.position.x < camX) enemy.position.x = camX;
                if (enemy.position.x + enemy.position.w > camX + camW) {
                    enemy.position.x = camX + camW - enemy.position.w;
                    enemy.direction = 1;
                }
            }
            moveObstacle(&spider);
            moveObstacle(&falling);
            updateSnowballs(snowballs, &snowballCount);


            SDL_Rect oldEPos = player.position;
            updatePlayer(&player, &enemy);
            if (player.position.x < 0) player.position.x = 0;
            if (player.position.y < 0) player.position.y = 0;
            if (player.position.x + player.position.w > SCREEN_WIDTH)
                player.position.x = SCREEN_WIDTH - player.position.w;
            if (player.position.y + player.position.h > SCREEN_HEIGHT)
                player.position.y = SCREEN_HEIGHT - player.position.h;

            checkSnowballEnemyCollision(snowballs, &snowballCount, &enemy, &hud);
            checkPlayerEnemyCollision(&player, &enemy);
            checkCollision(g.renderer, &enemy, &falling, collisionSound);
            checkCollision(g.renderer, &enemy, &spider, collisionSound);

            int pp = collisionPP(maskSurf, g.player1.rect);
            if (pp) {
                g.player1.rect          = oldPos;
                mmState.collisionPPEvent = pp;
                mmState.borderTimer      = 30;
                declencherEtincelle(&etincelle, mm.playerPosition, 0);
            }


            if (collisionBB(g.player1.rect, enemy.position)) {
                g.player1.rect = oldPos;
                enemy.state.playerTouch = 1;
                mmState.borderTimer = 30;
                declencherEtincelle(&etincelle, mm.playerPosition, 0);
            }


            if (collisionBB(player.position, enemy.position)) {
                player.position = oldEPos;
                enemy.state.playerTouch = 1;
            }

            mmState.posJoueur = g.player1.rect;
            if (mmState.borderTimer > 0)
                mmState.borderTimer--;
            updateEtincelle(&etincelle);
        } else {
            nextEnigmeTick = 0;
        }
        SDL_Delay(16);
    }
    SDL_StopTextInput();
    Liberation(&etincelle, NULL, maskSurf, &mm);
    /* Free enigme resources if loaded */
    if (enigmeOpen) E22_Liberation(&enigme);
    destroyAll(&enemy, &spider, &falling, collisionSound, &hud, &snowballs[0]);
    if (backgroundMusic) Mix_FreeMusic(backgroundMusic);
    CleanupSDL(&g);
    return 0;
}
