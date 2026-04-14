#include "bg.h"

/* ── helpers ────────────────────────────────────────────────────────── */

SDL_Texture *LoadTex(Game *g, const char *path) {
    SDL_Surface *s = IMG_Load(path);
    if (!s) { fprintf(stderr, "IMG_Load %s: %s\n", path, IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, s);
    SDL_FreeSurface(s); return t;
}

void DrawFilledRect(Game *g, SDL_Rect r, Uint8 R, Uint8 G, Uint8 B, Uint8 A) {
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, R, G, B, A);
    SDL_RenderFillRect(g->renderer, &r);
}

void DrawBorderRect(Game *g, SDL_Rect r, Uint8 R, Uint8 G, Uint8 B, Uint8 A) {
    SDL_SetRenderDrawBlendMode(g->renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(g->renderer, R, G, B, A);
    SDL_RenderDrawRect(g->renderer, &r);
}

void RenderText(Game *g, TTF_Font *f, const char *txt, SDL_Color c, int x, int y) {
    if (!f || !txt || !*txt) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, s);
    if (t) { SDL_Rect d = {x, y, s->w, s->h}; SDL_RenderCopy(g->renderer, t, NULL, &d); SDL_DestroyTexture(t); }
    SDL_FreeSurface(s);
}

void RenderTextW(Game *g, TTF_Font *f, const char *txt, SDL_Color c, int x, int y, int wrap) {
    if (!f || !txt) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended_Wrapped(f, txt, c, wrap);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, s);
    if (t) { SDL_Rect d = {x, y, s->w, s->h}; SDL_RenderCopy(g->renderer, t, NULL, &d); SDL_DestroyTexture(t); }
    SDL_FreeSurface(s);
}

void RenderTextCentered(Game *g, TTF_Font *f, const char *txt, SDL_Color c, SDL_Rect box) {
    if (!f || !txt || !*txt) return;
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, c);
    if (!s) return;
    SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, s);
    if (t) {
        SDL_Rect d = {box.x + box.w/2 - s->w/2, box.y + box.h/2 - s->h/2, s->w, s->h};
        SDL_RenderCopy(g->renderer, t, NULL, &d); SDL_DestroyTexture(t);
    }
    SDL_FreeSurface(s);
}

void ClampPlayer(Player *p, int lw, int lh) {
    if (p->rect.x < 0)              p->rect.x = 0;
    if (p->rect.y < 0)              p->rect.y = 0;
    if (p->rect.x + p->rect.w > lw) p->rect.x = lw - p->rect.w;
    if (p->rect.y + p->rect.h > lh) p->rect.y = lh - p->rect.h;
}

void SmoothCam(float *camX, int playerX, int camW, int levelW, int offset) {
    float target = (float)(playerX - offset);
    if (target < 0)              target = 0;
    if (target + camW > levelW)  target = (float)(levelW - camW);
    *camX += (target - *camX) * CAM_SPEED;
}

void GoToNameInput(Game *g, int split) {
    g->splitScreen = split; g->inputTarget = 1;
    g->inputLen = 0; memset(g->inputBuffer, 0, 64);
    g->state = STATE_NAME_INPUT;
}

/* ── init / cleanup ─────────────────────────────────────────────────── */

int InitSDL(Game *g, const char *title, int w, int h) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) { fprintf(stderr, "SDL_Init: %s\n", SDL_GetError()); return 0; }
    if (TTF_Init() != 0)               { fprintf(stderr, "TTF_Init: %s\n", TTF_GetError()); SDL_Quit(); return 0; }
    if (!(IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG) & (IMG_INIT_JPG|IMG_INIT_PNG))) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError()); TTF_Quit(); SDL_Quit(); return 0;
    }
    g->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    if (!g->window) { fprintf(stderr, "CreateWindow: %s\n", SDL_GetError()); return 0; }
    g->renderer = SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if (!g->renderer) { fprintf(stderr, "CreateRenderer: %s\n", SDL_GetError()); return 0; }
    g->font      = TTF_OpenFont("arial.ttf", 22);
    if (!g->font) g->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 22);
    g->fontLarge = TTF_OpenFont("arial.ttf", 36);
    if (!g->fontLarge) g->fontLarge = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 36);
    if (!g->font) { fprintf(stderr, "Font failed\n"); return 0; }
    g->startTime = SDL_GetTicks();
    return 1;
}

void CleanupSDL(Game *g) {
    if (g->font)             TTF_CloseFont(g->font);
    if (g->fontLarge)        TTF_CloseFont(g->fontLarge);
    if (g->background.image) SDL_DestroyTexture(g->background.image);
    if (g->nameBg)           SDL_DestroyTexture(g->nameBg);
    if (g->player1.sprite)   SDL_DestroyTexture(g->player1.sprite);
    if (g->player2.sprite)   SDL_DestroyTexture(g->player2.sprite);
    if (g->renderer)         SDL_DestroyRenderer(g->renderer);
    if (g->window)           SDL_DestroyWindow(g->window);
    if (g->platforms)        free(g->platforms);
    IMG_Quit(); TTF_Quit(); SDL_Quit();
}

int LoadBackground(Game *g, const char *path) {
    if (g->background.image) SDL_DestroyTexture(g->background.image);
    g->background.image = LoadTex(g, path);
    return g->background.image ? 1 : 0;
}

int LoadNameBackground(Game *g, const char *path) {
    if (g->nameBg) SDL_DestroyTexture(g->nameBg);
    g->nameBg = LoadTex(g, path);
    return g->nameBg ? 1 : 0;
}

void InitBackground(Game *g, int w, int h) {
    g->background.camera1   = (SDL_Rect){0, 0, w/2, h};
    g->background.camera2   = (SDL_Rect){0, 0, w/2, h};
    g->background.posEcran1 = (SDL_Rect){0, 0, w/2, h};
    g->background.posEcran2 = (SDL_Rect){w/2, 0, w/2, h};
    g->background.camX1 = g->background.camX2 = 0.0f;
}

void InitPlatforms(Game *g, int level) { (void)level; g->platformCount = 0; g->platforms = NULL; }

int InitPlayer(Player *p, SDL_Renderer *r, const char *path, int x, int y) {
    (void)r; (void)path;
    *p = (Player){0}; p->rect = (SDL_Rect){x, y, 48, 64}; p->lives = 3;
    return 1;
}

Uint32 GetElapsedTime(Game *g) { return SDL_GetTicks() - g->startTime; }
int    PointInRect(int x, int y, SDL_Rect r) { return x>=r.x && x<=r.x+r.w && y>=r.y && y<=r.y+r.h; }

/* ── render helpers ─────────────────────────────────────────────────── */

void RenderPlayerRect(Game *g, Player *p, SDL_Rect *cam) {
    SDL_Rect d = {p->rect.x - cam->x, p->rect.y - cam->y, p->rect.w, p->rect.h};
    SDL_SetRenderDrawColor(g->renderer,
        p==&g->player1 ? 50  : 255,
        p==&g->player1 ? 150 : 100,
        p==&g->player1 ? 255 : 100, 255);
    SDL_RenderFillRect(g->renderer, &d);
}

void RenderPlatformsOnCam(Game *g, SDL_Rect *cam) {
    for (int i = 0; i < g->platformCount; i++) {
        if (!g->platforms[i].active) continue;
        SDL_Rect d = g->platforms[i].rect; d.x -= cam->x; d.y -= cam->y;
        SDL_SetRenderDrawColor(g->renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(g->renderer, &d);
    }
}

void RenderTime(Game *g, SDL_Rect *pos) {
    char buf[32]; Uint32 ms = GetElapsedTime(g); int s = ms/1000;
    snprintf(buf, 32, "Time: %02d:%02d", s/60, s%60);
    RenderText(g, g->font, buf, (SDL_Color){255,255,255,255}, pos->x+10, pos->y+10);
}

/* ── render states ──────────────────────────────────────────────────── */

void RenderMenu(Game *g) {
    SDL_SetRenderDrawColor(g->renderer, 20, 20, 40, 255);
    SDL_RenderClear(g->renderer);
    SDL_Color w = {255,255,255,255};
    SDL_Rect b1 = {SCREEN_WIDTH/2-200, 250, 400, 80};
    SDL_Rect b2 = {SCREEN_WIDTH/2-200, 370, 400, 80};
    DrawFilledRect(g, b1, 50, 150, 255, 255);
    DrawFilledRect(g, b2, 255, 100, 100, 255);
    RenderText(g, g->fontLarge ? g->fontLarge : g->font, "MON JEU SDL2", w, SCREEN_WIDTH/2-110, 140);
    RenderTextCentered(g, g->font, "Ecran Unique", w, b1);
    RenderTextCentered(g, g->font, "Ecran Divise", w, b2);
    SDL_RenderPresent(g->renderer);
}

void RenderNameInput(Game *g) {
    SDL_RenderClear(g->renderer);
    if (g->nameBg) SDL_RenderCopy(g->renderer, g->nameBg, NULL, NULL);
    SDL_Color green = {0,220,0,255}, black = {0,0,0,255}, white = {255,255,255,255};
    TTF_Font *big = g->fontLarge ? g->fontLarge : g->font;
    const char *label = g->inputTarget==1
        ? "Veuillez saisir le nom du Joueur 1 :"
        : "Veuillez saisir le nom du Joueur 2 :";
    SDL_Surface *ls = TTF_RenderUTF8_Blended(big, label, green);
    if (ls) {
        SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, ls);
        SDL_Rect d = {SCREEN_WIDTH/2 - ls->w/2, 220, ls->w, ls->h};
        SDL_RenderCopy(g->renderer, t, NULL, &d);
        SDL_FreeSurface(ls); SDL_DestroyTexture(t);
    }
    SDL_Rect box = {SCREEN_WIDTH/2-300, 300, 600, 70};
    DrawFilledRect(g, box, 255, 255, 255, 255);
    DrawBorderRect(g, box, 0, 0, 0, 255);
    if (g->inputLen > 0) RenderTextCentered(g, big, g->inputBuffer, black, box);
    RenderText(g, g->font, "Appuyez sur ENTREE pour valider", white, SCREEN_WIDTH/2-160, 400);
    SDL_RenderPresent(g->renderer);
}

void RenderGuideOverlay(Game *g) {
    int gw=700, gh=460, gx=SCREEN_WIDTH/2-350, gy=SCREEN_HEIGHT/2-230;
    SDL_Color w={255,255,255,255}, y={255,220,50,255}, c={80,220,255,255};
    DrawFilledRect(g, (SDL_Rect){0,0,SCREEN_WIDTH,SCREEN_HEIGHT}, 0,0,0,160);
    DrawFilledRect(g, (SDL_Rect){gx,gy,gw,gh}, 15,20,45,235);
    DrawBorderRect(g, (SDL_Rect){gx,gy,gw,gh}, 100,180,255,255);
    TTF_Font *big = g->fontLarge ? g->fontLarge : g->font;
    SDL_Surface *ts = TTF_RenderUTF8_Blended(big, "Guide du Jeu", y);
    if (ts) {
        SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, ts);
        SDL_Rect d = {gx+gw/2-ts->w/2, gy+18, ts->w, ts->h};
        SDL_RenderCopy(g->renderer, t, NULL, &d);
        SDL_FreeSurface(ts); SDL_DestroyTexture(t);
    }
    SDL_SetRenderDrawColor(g->renderer, 100,180,255,180);
    SDL_RenderDrawLine(g->renderer, gx+20, gy+68, gx+gw-20, gy+68);
    RenderText(g, g->font, "CONTROLES",                      c, gx+30, gy+82);
    RenderText(g, g->font, "Joueur 1 (Bleu):",                w, gx+30, gy+114);
    RenderText(g, g->font, "W - Haut    S - Bas",             w, gx+50, gy+140);
    RenderText(g, g->font, "A - Gauche  D - Droite",          w, gx+50, gy+165);
    RenderText(g, g->font, "Joueur 2 (Rouge):",                w, gx+30, gy+200);
    RenderText(g, g->font, "Haut / Bas / Gauche / Droite (fleches)", w, gx+50, gy+226);
    SDL_SetRenderDrawColor(g->renderer, 100,180,255,100);
    SDL_RenderDrawLine(g->renderer, gx+20, gy+268, gx+gw-20, gy+268);
    RenderText(g, g->font, "OBJECTIF", c, gx+30, gy+282);
    RenderTextW(g, g->font,
        "Attrapez Kevin tout en collectant les objets dans la maison, "
        "sans vous faire toucher ou attaquer par l'ennemi !",
        w, gx+30, gy+310, gw-60);
    SDL_Rect btn = {gx+gw-180, gy+gh-50, 160, 36};
    g->guideBtnClose = btn;
    DrawFilledRect(g, btn, 200,60,60,230);
    DrawBorderRect(g, btn, 255,120,120,255);
    RenderTextCentered(g, g->font, "Retour [ESC]", w, btn);
}

void RenderGameSingle(Game *g, int keys[]) {
    (void)keys;
    SDL_RenderClear(g->renderer);
    SDL_RenderCopy(g->renderer, g->background.image, &g->background.camera1, NULL);
    RenderPlatformsOnCam(g, &g->background.camera1);
    RenderPlayerRect(g, &g->player1, &g->background.camera1);
    RenderPlayerRect(g, &g->player2, &g->background.camera1);
    RenderTime(g, &g->background.posEcran1);
    if (g->showGuide) RenderGuideOverlay(g);
    // SDL_RenderPresent(g->renderer); // Supprimé pour éviter le double affichage
}

void RenderGameSplit(Game *g, int keys[]) {
    (void)keys;
    SDL_Rect vp1 = g->background.posEcran1, vp2 = g->background.posEcran2;
    SDL_RenderClear(g->renderer);
    SDL_RenderSetViewport(g->renderer, &vp1);
    SDL_RenderCopy(g->renderer, g->background.image, &g->background.camera1, NULL);
    RenderPlatformsOnCam(g, &g->background.camera1);
    RenderPlayerRect(g, &g->player1, &g->background.camera1);
    SDL_RenderSetViewport(g->renderer, &vp2);
    SDL_RenderCopy(g->renderer, g->background.image, &g->background.camera2, NULL);
    RenderPlatformsOnCam(g, &g->background.camera2);
    RenderPlayerRect(g, &g->player2, &g->background.camera2);
    SDL_RenderSetViewport(g->renderer, NULL);
    SDL_SetRenderDrawColor(g->renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(g->renderer, SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT);
    RenderTime(g, &g->background.posEcran1);
    RenderTime(g, &g->background.posEcran2);
    if (g->showGuide) RenderGuideOverlay(g);
    // SDL_RenderPresent(g->renderer); // Supprimé pour éviter le double affichage
}

/* ── event handlers ─────────────────────────────────────────────────── */

void HandleMenuClick(Game *g, int mx, int my) {
    SDL_Rect b1 = {SCREEN_WIDTH/2-200, 250, 400, 80};
    SDL_Rect b2 = {SCREEN_WIDTH/2-200, 370, 400, 80};
    if (PointInRect(mx, my, b1)) GoToNameInput(g, 0);
    if (PointInRect(mx, my, b2)) GoToNameInput(g, 1);
}

void HandleNameInput(Game *g, SDL_Event *e) {
    if (e->type == SDL_KEYDOWN) {
        SDL_Keycode k = e->key.keysym.sym;
        if (k == SDLK_RETURN && g->inputLen > 0) {
            if (g->inputTarget == 1) {
                strncpy(g->player1.name, g->inputBuffer, 63);
                g->inputTarget = 2; g->inputLen = 0; memset(g->inputBuffer, 0, 64);
            } else {
                strncpy(g->player2.name, g->inputBuffer, 63);
                g->state = STATE_GAME;
            }
        } else if (k == SDLK_BACKSPACE && g->inputLen > 0) {
            g->inputBuffer[--g->inputLen] = '\0';
        } else if (k == SDLK_ESCAPE) {
            g->state = STATE_MENU;
        }
    } else if (e->type == SDL_TEXTINPUT) {
        int n = (int)strlen(e->text.text);
        if (g->inputLen + n < 63) { strcat(g->inputBuffer, e->text.text); g->inputLen += n; }
    }
}

void HandleGuideClick(Game *g, int mx, int my) {
    if (g->showGuide && PointInRect(mx, my, g->guideBtnClose)) g->showGuide = 0;
}

/* ── update ─────────────────────────────────────────────────────────── */

void UpdatePlayersSingle(Game *g, int keys[], int lw, int lh) {
    if (g->showGuide) return;
    if (keys[SDL_SCANCODE_D]) g->player1.rect.x += VITESSE;
    if (keys[SDL_SCANCODE_A]) g->player1.rect.x -= VITESSE;
    if (keys[SDL_SCANCODE_W]) g->player1.rect.y -= VITESSE;
    if (keys[SDL_SCANCODE_S]) g->player1.rect.y += VITESSE;
    if (keys[SDL_SCANCODE_RIGHT]) g->player2.rect.x += VITESSE;
    if (keys[SDL_SCANCODE_LEFT])  g->player2.rect.x -= VITESSE;
    if (keys[SDL_SCANCODE_UP])    g->player2.rect.y -= VITESSE;
    if (keys[SDL_SCANCODE_DOWN])  g->player2.rect.y += VITESSE;
    ClampPlayer(&g->player1, lw, lh);
    ClampPlayer(&g->player2, lw, lh);
    SmoothCam(&g->background.camX1, g->player1.rect.x, SCREEN_WIDTH, lw, SCREEN_WIDTH/2);
    g->background.camera1   = (SDL_Rect){(int)g->background.camX1, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    g->background.posEcran1 = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
}

void UpdatePlayersSplit(Game *g, int keys[], int lw, int lh) {
    if (g->showGuide) return;
    int half = SCREEN_WIDTH/2;
    if (keys[SDL_SCANCODE_D]) g->player1.rect.x += VITESSE;
    if (keys[SDL_SCANCODE_A]) g->player1.rect.x -= VITESSE;
    if (keys[SDL_SCANCODE_W]) g->player1.rect.y -= VITESSE;
    if (keys[SDL_SCANCODE_S]) g->player1.rect.y += VITESSE;
    if (keys[SDL_SCANCODE_RIGHT]) g->player2.rect.x += VITESSE;
    if (keys[SDL_SCANCODE_LEFT])  g->player2.rect.x -= VITESSE;
    if (keys[SDL_SCANCODE_UP])    g->player2.rect.y -= VITESSE;
    if (keys[SDL_SCANCODE_DOWN])  g->player2.rect.y += VITESSE;
    ClampPlayer(&g->player1, lw, lh);
    ClampPlayer(&g->player2, lw, lh);
    SmoothCam(&g->background.camX1, g->player1.rect.x, half, lw, half/2);
    SmoothCam(&g->background.camX2, g->player2.rect.x, half, lw, half/2);
    g->background.camera1   = (SDL_Rect){(int)g->background.camX1, 0, half, SCREEN_HEIGHT};
    g->background.camera2   = (SDL_Rect){(int)g->background.camX2, 0, half, SCREEN_HEIGHT};
    g->background.posEcran1 = (SDL_Rect){0,    0, half, SCREEN_HEIGHT};
    g->background.posEcran2 = (SDL_Rect){half, 0, half, SCREEN_HEIGHT};
}

