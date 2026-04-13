#include "header.h"

SDL_Texture *loadTex(Game *g, const char *p) {
    SDL_Surface *s = IMG_Load(p);
    if (!s) { fprintf(stderr, "IMG_Load %s: %s\n", p, IMG_GetError()); return NULL; }
    SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, s);
    SDL_FreeSurface(s); return t;
}

void putRect(SDL_Renderer *r, SDL_Rect R, Uint8 a, Uint8 b, Uint8 c, Uint8 d, int fill) {
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, a, b, c, d);
    if (fill) SDL_RenderFillRect(r, &R); else SDL_RenderDrawRect(r, &R);
}

void putText(Game *g, TTF_Font *f, const char *t, SDL_Color c, int x, int y, int wrap, int centered, SDL_Rect box) {
    if (!f || !t || !*t) return;
    SDL_Surface *s = wrap ? TTF_RenderUTF8_Blended_Wrapped(f, t, c, wrap)
                          : TTF_RenderUTF8_Blended(f, t, c);
    if (!s) return;
    SDL_Texture *tx = SDL_CreateTextureFromSurface(g->renderer, s);
    if (tx) {
        SDL_Rect d = centered
            ? (SDL_Rect){box.x+box.w/2-s->w/2, box.y+box.h/2-s->h/2, s->w, s->h}
            : (SDL_Rect){x, y, s->w, s->h};
        SDL_RenderCopy(g->renderer, tx, NULL, &d);
        SDL_DestroyTexture(tx);
    }
    SDL_FreeSurface(s);
}

void clamp(Player *p, int lw, int lh) {
    if (p->rect.x < 0)              p->rect.x = 0;
    if (p->rect.y < 0)              p->rect.y = 0;
    if (p->rect.x + p->rect.w > lw) p->rect.x = lw - p->rect.w;
    if (p->rect.y + p->rect.h > lh) p->rect.y = lh - p->rect.h;
}

void smoothCam(float *cx, int px, int cw, int lw, int off) {
    float t = (float)(px - off);
    if (t < 0) t = 0;
    if (t + cw > lw) t = (float)(lw - cw);
    *cx += (t - *cx) * CAM_SPEED;
}

void renderBg(Game *g, SDL_Rect cam, SDL_Rect vp) {
    SDL_RenderSetViewport(g->renderer, &vp);
    SDL_RenderCopy(g->renderer, g->background.image, &cam, NULL);
}

void renderPlayer(Game *g, Player *p, SDL_Rect cam) {
    SDL_Rect d = {p->rect.x-cam.x, p->rect.y-cam.y, p->rect.w, p->rect.h};
    SDL_SetRenderDrawColor(g->renderer,
        p==&g->player1?50:255, p==&g->player1?150:100, p==&g->player1?255:100, 255);
    SDL_RenderFillRect(g->renderer, &d);
}

void renderTime(Game *g, int x, int y) {
    char buf[32]; int s = (SDL_GetTicks()-g->startTime)/1000;
    snprintf(buf, 32, "Time: %02d:%02d", s/60, s%60);
    putText(g, g->font, buf, (SDL_Color){255,255,255,255}, x+10, y+10, 0, 0, (SDL_Rect){0,0,0,0});
}

void renderGuide(Game *g) {
    int gw=700, gh=460, gx=SCREEN_WIDTH/2-350, gy=SCREEN_HEIGHT/2-230;
    SDL_Color w={255,255,255,255}, y={255,220,50,255}, c={80,220,255,255};
    putRect(g->renderer, (SDL_Rect){0,0,SCREEN_WIDTH,SCREEN_HEIGHT}, 0,0,0,160, 1);
    putRect(g->renderer, (SDL_Rect){gx,gy,gw,gh}, 15,20,45,235, 1);
    putRect(g->renderer, (SDL_Rect){gx,gy,gw,gh}, 100,180,255,255, 0);
    TTF_Font *big = g->fontLarge ? g->fontLarge : g->font;
    putText(g, big, "Guide du Jeu", y, gx+gw/2, gy+18, 0, 0, (SDL_Rect){0,0,0,0});
    SDL_SetRenderDrawColor(g->renderer, 100,180,255,180);
    SDL_RenderDrawLine(g->renderer, gx+20, gy+68, gx+gw-20, gy+68);
    putText(g, g->font, "CONTROLES",                                     c, gx+30,gy+82,  0,0,(SDL_Rect){0,0,0,0});
    putText(g, g->font, "Joueur 1: W-Haut  S-Bas  A-Gauche  D-Droite",  w, gx+30,gy+114, 0,0,(SDL_Rect){0,0,0,0});
    putText(g, g->font, "Joueur 2: Fleches Haut/Bas/Gauche/Droite",      w, gx+30,gy+148, 0,0,(SDL_Rect){0,0,0,0});
    SDL_SetRenderDrawColor(g->renderer, 100,180,255,100);
    SDL_RenderDrawLine(g->renderer, gx+20, gy+190, gx+gw-20, gy+190);
    putText(g, g->font, "OBJECTIF", c, gx+30,gy+205, 0,0,(SDL_Rect){0,0,0,0});
    putText(g, g->font,
        "Attrapez Kevin tout en collectant les objets dans la maison, "
        "sans vous faire toucher ou attaquer par l'ennemi !",
        w, gx+30, gy+235, gw-60, 0, (SDL_Rect){0,0,0,0});
    SDL_Rect btn = {gx+gw-180, gy+gh-50, 160, 36};
    g->guideBtnClose = btn;
    putRect(g->renderer, btn, 200,60,60,230, 1);
    putRect(g->renderer, btn, 255,120,120,255, 0);
    putText(g, g->font, "Retour [ESC]", w, 0,0, 0,1, btn);
}

int InitGame(Game *g, const char *title, int w, int h) {
    memset(g, 0, sizeof(Game));
    g->state = STATE_MENU; g->running = 1;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)  { fprintf(stderr, "SDL_Init: %s\n",  SDL_GetError()); return 0; }
    if (TTF_Init() != 0)                { fprintf(stderr, "TTF_Init: %s\n",  TTF_GetError()); return 0; }
    if (!(IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG) & (IMG_INIT_JPG|IMG_INIT_PNG))) { fprintf(stderr, "IMG_Init\n"); return 0; }
    g->window   = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    g->renderer = g->window ? SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC) : NULL;
    if (!g->renderer) { fprintf(stderr, "Renderer: %s\n", SDL_GetError()); return 0; }
    g->font      = TTF_OpenFont("arial.ttf", 22);
    if (!g->font) g->font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 22);
    g->fontLarge = TTF_OpenFont("arial.ttf", 36);
    if (!g->fontLarge) g->fontLarge = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 36);
    if (!g->font) { fprintf(stderr, "Font failed\n"); return 0; }
    g->startTime = SDL_GetTicks();
    SDL_StartTextInput();
    return 1;
}

int LoadRessources(Game *g, const char *bgPath, const char *nameBgPath) {
    g->background.image = loadTex(g, bgPath);
    g->nameBg           = loadTex(g, nameBgPath);
    g->background.camera1   = (SDL_Rect){0, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT};
    g->background.camera2   = (SDL_Rect){0, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT};
    g->background.posEcran1 = (SDL_Rect){0, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT};
    g->background.posEcran2 = (SDL_Rect){SCREEN_WIDTH/2, 0, SCREEN_WIDTH/2, SCREEN_HEIGHT};
    g->background.camX1 = g->background.camX2 = 0.0f;
    g->player1 = (Player){0}; g->player1.rect = (SDL_Rect){100,  300, 48, 64}; g->player1.lives = 3;
    g->player2 = (Player){0}; g->player2.rect = (SDL_Rect){1000, 300, 48, 64}; g->player2.lives = 3;
    return g->background.image ? 1 : 0;
}

void HandleEvents(Game *g, int keys[]) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { g->running = 0; return; }

        if (g->state == STATE_MENU && e.type == SDL_MOUSEBUTTONDOWN) {
            SDL_Rect b1={SCREEN_WIDTH/2-200,250,400,80}, b2={SCREEN_WIDTH/2-200,370,400,80};
            int mx=e.button.x, my=e.button.y;
            if (mx>=b1.x&&mx<=b1.x+b1.w&&my>=b1.y&&my<=b1.y+b1.h) { g->splitScreen=0; g->inputTarget=1; g->inputLen=0; memset(g->inputBuffer,0,64); g->state=STATE_NAME_INPUT; }
            if (mx>=b2.x&&mx<=b2.x+b2.w&&my>=b2.y&&my<=b2.y+b2.h) { g->splitScreen=1; g->inputTarget=1; g->inputLen=0; memset(g->inputBuffer,0,64); g->state=STATE_NAME_INPUT; }
        }
        else if (g->state == STATE_NAME_INPUT) {
            if (e.type==SDL_KEYDOWN) {
                SDL_Keycode k = e.key.keysym.sym;
                if (k==SDLK_RETURN && g->inputLen>0) {
                    if (g->inputTarget==1) { strncpy(g->player1.name,g->inputBuffer,63); g->inputTarget=2; g->inputLen=0; memset(g->inputBuffer,0,64); }
                    else                   { strncpy(g->player2.name,g->inputBuffer,63); g->state=STATE_GAME; }
                }
                else if (k==SDLK_BACKSPACE && g->inputLen>0) g->inputBuffer[--g->inputLen]='\0';
                else if (k==SDLK_ESCAPE) g->state=STATE_MENU;
            }
            else if (e.type==SDL_TEXTINPUT) {
                int n=(int)strlen(e.text.text);
                if (g->inputLen+n<63) { strcat(g->inputBuffer,e.text.text); g->inputLen+=n; }
            }
        }
        else if (g->state == STATE_GAME) {
            if (e.type==SDL_MOUSEBUTTONDOWN && g->showGuide) {
                int mx=e.button.x, my=e.button.y; SDL_Rect b=g->guideBtnClose;
                if (mx>=b.x&&mx<=b.x+b.w&&my>=b.y&&my<=b.y+b.h) g->showGuide=0;
            }
            if (e.type==SDL_KEYDOWN) {
                SDL_Scancode sc=e.key.keysym.scancode;
                if      (sc==SDL_SCANCODE_H||sc==SDL_SCANCODE_F1) g->showGuide=!g->showGuide;
                else if (sc==SDL_SCANCODE_ESCAPE) { if(g->showGuide) g->showGuide=0; else { memset(keys,0,SDL_NUM_SCANCODES*sizeof(int)); g->state=STATE_MENU; } }
                else keys[sc]=1;
            }
            if (e.type==SDL_KEYUP) keys[e.key.keysym.scancode]=0;
        }
    }
}

void Update(Game *g, int keys[]) {
    if (g->state != STATE_GAME || g->showGuide) return;
    int lw=LEVEL_WIDTH, lh=LEVEL_HEIGHT, half=SCREEN_WIDTH/2;
    if (keys[SDL_SCANCODE_D]) g->player1.rect.x += VITESSE;
    if (keys[SDL_SCANCODE_A]) g->player1.rect.x -= VITESSE;
    if (keys[SDL_SCANCODE_W]) g->player1.rect.y -= VITESSE;
    if (keys[SDL_SCANCODE_S]) g->player1.rect.y += VITESSE;
    if (keys[SDL_SCANCODE_RIGHT]) g->player2.rect.x += VITESSE;
    if (keys[SDL_SCANCODE_LEFT])  g->player2.rect.x -= VITESSE;
    if (keys[SDL_SCANCODE_UP])    g->player2.rect.y -= VITESSE;
    if (keys[SDL_SCANCODE_DOWN])  g->player2.rect.y += VITESSE;
    clamp(&g->player1, lw, lh);
    clamp(&g->player2, lw, lh);
    if (g->splitScreen) {
        smoothCam(&g->background.camX1, g->player1.rect.x, half, lw, half/2);
        smoothCam(&g->background.camX2, g->player2.rect.x, half, lw, half/2);
        g->background.camera1   = (SDL_Rect){(int)g->background.camX1, 0, half, SCREEN_HEIGHT};
        g->background.camera2   = (SDL_Rect){(int)g->background.camX2, 0, half, SCREEN_HEIGHT};
        g->background.posEcran1 = (SDL_Rect){0,    0, half, SCREEN_HEIGHT};
        g->background.posEcran2 = (SDL_Rect){half, 0, half, SCREEN_HEIGHT};
    } else {
        smoothCam(&g->background.camX1, g->player1.rect.x, SCREEN_WIDTH, lw, SCREEN_WIDTH/2);
        g->background.camera1   = (SDL_Rect){(int)g->background.camX1, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        g->background.posEcran1 = (SDL_Rect){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    }
}

void Render(Game *g, int keys[]) {
    (void)keys;
    SDL_RenderClear(g->renderer);

    if (g->state == STATE_MENU) {
        SDL_Color w={255,255,255,255};
        SDL_Rect b1={SCREEN_WIDTH/2-200,250,400,80}, b2={SCREEN_WIDTH/2-200,370,400,80};
        putRect(g->renderer, (SDL_Rect){0,0,SCREEN_WIDTH,SCREEN_HEIGHT}, 20,20,40,255, 1);
        putRect(g->renderer, b1, 50,150,255,255, 1);
        putRect(g->renderer, b2, 255,100,100,255, 1);
        putText(g, g->fontLarge?g->fontLarge:g->font, "MON JEU SDL2", w, SCREEN_WIDTH/2-110,140, 0,0,(SDL_Rect){0,0,0,0});
        putText(g, g->font, "Ecran Unique", w, 0,0, 0,1, b1);
        putText(g, g->font, "Ecran Divise", w, 0,0, 0,1, b2);
    }
    else if (g->state == STATE_NAME_INPUT) {
        SDL_Color green={0,220,0,255}, black={0,0,0,255}, white={255,255,255,255};
        TTF_Font *big = g->fontLarge?g->fontLarge:g->font;
        if (g->nameBg) SDL_RenderCopy(g->renderer, g->nameBg, NULL, NULL);
        const char *lbl = g->inputTarget==1 ? "Veuillez saisir le nom du Joueur 1 :"
                                             : "Veuillez saisir le nom du Joueur 2 :";
        SDL_Surface *ls = TTF_RenderUTF8_Blended(big, lbl, green);
        if (ls) {
            SDL_Texture *t = SDL_CreateTextureFromSurface(g->renderer, ls);
            SDL_Rect d = {SCREEN_WIDTH/2-ls->w/2, 220, ls->w, ls->h};
            SDL_RenderCopy(g->renderer, t, NULL, &d);
            SDL_FreeSurface(ls); SDL_DestroyTexture(t);
        }
        SDL_Rect box = {SCREEN_WIDTH/2-300, 300, 600, 70};
        putRect(g->renderer, box, 255,255,255,255, 1);
        putRect(g->renderer, box, 0,0,0,255, 0);
        if (g->inputLen>0) putText(g, big, g->inputBuffer, black, 0,0, 0,1, box);
        putText(g, g->font, "Appuyez sur ENTREE pour valider", white, SCREEN_WIDTH/2-160,400, 0,0,(SDL_Rect){0,0,0,0});
    }
    else if (g->state == STATE_GAME) {
        SDL_Rect full = {0,0,SCREEN_WIDTH,SCREEN_HEIGHT};
        if (g->splitScreen) {
            renderBg(g, g->background.camera1, g->background.posEcran1);
            renderPlayer(g, &g->player1, g->background.camera1);
            renderBg(g, g->background.camera2, g->background.posEcran2);
            renderPlayer(g, &g->player2, g->background.camera2);
            SDL_RenderSetViewport(g->renderer, NULL);
            SDL_SetRenderDrawColor(g->renderer, 255,255,255,255);
            SDL_RenderDrawLine(g->renderer, SCREEN_WIDTH/2,0, SCREEN_WIDTH/2,SCREEN_HEIGHT);
            renderTime(g, g->background.posEcran1.x, g->background.posEcran1.y);
            renderTime(g, g->background.posEcran2.x, g->background.posEcran2.y);
        } else {
            renderBg(g, g->background.camera1, full);
            SDL_RenderSetViewport(g->renderer, NULL);
            renderPlayer(g, &g->player1, g->background.camera1);
            renderPlayer(g, &g->player2, g->background.camera1);
            renderTime(g, 0, 0);
        }
        if (g->showGuide) renderGuide(g);
    }
    SDL_RenderPresent(g->renderer);
}

void Cleanup(Game *g) {
    SDL_StopTextInput();
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
