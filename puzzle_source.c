/*
 * puzzle_source.c
 * Code du puzzle integre dans le projet Quiz.
 * La fonction RunPuzzle() remplace l'ancien main() du puzzle.
 * Elle cree sa propre fenetre SDL, tourne jusqu'a la fin,
 * puis libere tout et rend la main au menu du quiz.
 */
#include "header.h"

int inside(SDL_Rect *r, int x, int y) {
    return x>=r->x && x<r->x+r->w && y>=r->y && y<r->y+r->h;
}

int overlap(SDL_Rect *a, SDL_Rect *b) {
    return !(a->x+a->w<b->x || b->x+b->w<a->x || a->y+a->h<b->y || b->y+b->h<a->y);
}

void reset_pieces(Level *lv) {
    int ord[3]={0,1,2};
    for (int i=2;i>0;i--) { int j=rand()%(i+1),tmp=ord[i]; ord[i]=ord[j]; ord[j]=tmp; }
    for (int i=0;i<3;i++) {
        lv->pieces[ord[i]].home = (SDL_Rect){WIN_W-165, 80+i*160, 140, 140};
        lv->pieces[ord[i]].dst  = lv->pieces[ord[i]].home;
        lv->pieces[ord[i]].dragging = 0;
    }
}

int game_init(Game *g) {
    srand((unsigned)time(NULL));
    *g = (Game){0};
    /* SDL est deja initialise par le quiz : on cree juste une nouvelle fenetre */
    g->win = SDL_CreateWindow("Puzzle",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    g->ren = SDL_CreateRenderer(g->win, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    g->font      = TTF_OpenFont("arial.ttf", 52);
    g->music     = Mix_LoadMUS("music.mp3");
    g->snd_ok    = Mix_LoadWAV("beep2.wav");
    g->snd_wrong = Mix_LoadWAV("laugh.wav");
    if (g->music) { Mix_VolumeMusic(64); Mix_PlayMusic(g->music, -1); }
    return g->win && g->ren;
}

void game_load(Game *g) {
    int pw = WIN_W-180, ph = WIN_H-50;
    g->background = IMG_LoadTexture(g->ren, "bg2.png");

    Level *l0 = &g->levels[0];
    l0->bg = IMG_LoadTexture(g->ren, "picture1.png");
    l0->hole = (SDL_Rect){ (int)(687.f/979*pw), 50+(int)(114.f/645*ph), (int)(98.f/979*pw), (int)(99.f/645*ph) };
    l0->pieces[0] = (Piece){ IMG_LoadTexture(g->ren,"correct1.png"), {0},{0}, 1 };
    l0->pieces[1] = (Piece){ IMG_LoadTexture(g->ren,"wrong1.png"),   {0},{0}, 0 };
    l0->pieces[2] = (Piece){ IMG_LoadTexture(g->ren,"wrong11.png"),  {0},{0}, 0 };
    reset_pieces(l0);

    Level *l1 = &g->levels[1];
    l1->bg = IMG_LoadTexture(g->ren, "picture2.png");
    l1->hole = (SDL_Rect){ (int)(524.f/1196*pw), 50+(int)(371.f/896*ph), (int)(151.f/1196*pw), (int)(152.f/896*ph) };
    l1->pieces[0] = (Piece){ IMG_LoadTexture(g->ren,"correct2.png"), {0},{0}, 1 };
    l1->pieces[1] = (Piece){ IMG_LoadTexture(g->ren,"wrong2.png"),   {0},{0}, 0 };
    l1->pieces[2] = (Piece){ IMG_LoadTexture(g->ren,"wrong22.png"),  {0},{0}, 0 };
    reset_pieces(l1);

    g->cur = 0;
    g->t0  = SDL_GetTicks();
}

void game_event(Game *g, SDL_Event *e) {
    if (g->solved || g->gameover) return;
    Level *lv = &g->levels[g->cur];

    if (e->type == SDL_MOUSEBUTTONDOWN)
        for (int i=0;i<3;i++)
            if (inside(&lv->pieces[i].dst, e->button.x, e->button.y)) {
                lv->pieces[i].dragging = 1;
                lv->pieces[i].ox = e->button.x - lv->pieces[i].dst.x;
                lv->pieces[i].oy = e->button.y - lv->pieces[i].dst.y;
            }

    if (e->type == SDL_MOUSEMOTION)
        for (int i=0;i<3;i++)
            if (lv->pieces[i].dragging) {
                lv->pieces[i].dst.x = e->motion.x - lv->pieces[i].ox;
                lv->pieces[i].dst.y = e->motion.y - lv->pieces[i].oy;
            }

    if (e->type == SDL_MOUSEBUTTONUP)
        for (int i=0;i<3;i++) {
            Piece *p = &lv->pieces[i];
            if (!p->dragging) continue;
            p->dragging = 0;
            if (overlap(&p->dst, &lv->hole)) {
                if (p->correct) {
                    p->dst       = lv->hole;
                    g->solved    = 1;
                    g->solved_at = SDL_GetTicks();
                    if (g->cur == NUM_LVL-1 && g->snd_ok)
                        Mix_PlayChannel(-1, g->snd_ok, 0);
                } else {
                    p->dst         = p->home;
                    g->wrong_flash = 90;
                    if (g->snd_wrong) Mix_PlayChannel(-1, g->snd_wrong, 0);
                }
            } else {
                p->dst = p->home;
            }
        }
}

void game_update(Game *g) {
    if (g->wrong_flash > 0) g->wrong_flash--;
    if (g->gameover) return;
    if (g->solved) {
        if (SDL_GetTicks()-g->solved_at > 1500) {
            if (g->cur+1 < NUM_LVL) {
                g->cur++;
                g->t0     = SDL_GetTicks();
                g->solved = 0;
                reset_pieces(&g->levels[g->cur]);
            }
        }
        return;
    }
    if ((int)(SDL_GetTicks()-g->t0) >= TIMER_SEC*1000)
        g->gameover = 1;
}

void game_draw(Game *g) {
    SDL_Renderer *ren = g->ren;
    Level *lv = &g->levels[g->cur];

    SDL_Rect full = {0,0,WIN_W,WIN_H};
    SDL_RenderCopy(ren, g->background, NULL, &full);

    SDL_Rect bg_dst = {0, 50, WIN_W-180, WIN_H-50};
    SDL_RenderCopy(ren, lv->bg, NULL, &bg_dst);

    if (!g->solved) {
        SDL_SetRenderDrawColor(ren, 0,0,0,255);
        SDL_RenderFillRect(ren, &lv->hole);
        SDL_SetRenderDrawColor(ren, 255,210,0,255);
        SDL_RenderDrawRect(ren, &lv->hole);
    }

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 10,10,20,200);
    SDL_Rect panel = {WIN_W-175, 45, 175, WIN_H-45};
    SDL_RenderFillRect(ren, &panel);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

    for (int i=0;i<3;i++) {
        Piece *p = &lv->pieces[i];
        if (p->dragging) {
            SDL_SetRenderDrawColor(ren, 255,210,0,255);
            SDL_Rect b = {p->dst.x-3, p->dst.y-3, p->dst.w+6, p->dst.h+6};
            SDL_RenderFillRect(ren, &b);
        }
        SDL_RenderCopy(ren, p->tex, NULL, &p->dst);
    }

    float ratio = 1.0f - (float)(SDL_GetTicks()-g->t0)/(TIMER_SEC*1000.f);
    if (ratio < 0) ratio = 0;

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 150);
    SDL_Rect bg = {0, 0, WIN_W, 50};
    SDL_RenderFillRect(ren, &bg);

    int max_w = WIN_W - 40;
    int h = 16, x = 20, y = 25;
    int w = (int)(max_w * ratio);

    Uint8 red = 255;
    Uint8 green = (Uint8)(220 * ratio);
    Uint8 blue  = (Uint8)(150 * ratio);
    SDL_SetRenderDrawColor(ren, red, green, blue, 255);
    SDL_Rect candle = {x, y - h/2, w, h};
    SDL_RenderFillRect(ren, &candle);

    if (ratio > 0) {
        int fx = x + w;
        int flicker = rand() % 3;
        SDL_SetRenderDrawColor(ren, 255, 140, 0, 255);
        SDL_Rect flame = {fx - 2, y - 8 - flicker, 5, 8};
        SDL_RenderFillRect(ren, &flame);
    }

    if (g->font) {
        SDL_Surface *surf = NULL;
        SDL_Texture *tex  = NULL;
        SDL_Rect dst;
        double angle = SDL_GetTicks() / 10.0;
        double zoom  = 1.0 + 0.15 * sin(SDL_GetTicks() / 200.0);

        if (g->wrong_flash > 0) {
            surf = TTF_RenderText_Blended(g->font, "WRONG!", (SDL_Color){220,30,30,255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ WIN_W/2-(int)(surf->w*zoom/2), WIN_H/2-(int)(surf->h*zoom/2), (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        }
        if (g->solved && g->cur == NUM_LVL-1) {
            surf = TTF_RenderText_Blended(g->font, "PUZZLE OK!", (SDL_Color){50,220,80,255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ WIN_W/2-(int)(surf->w*zoom/2), WIN_H/2-(int)(surf->h*zoom/2), (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        } else if (g->solved) {
            surf = TTF_RenderText_Blended(g->font, "NEXT!", (SDL_Color){50,180,255,255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ WIN_W/2-(int)(surf->w*zoom/2), WIN_H/2-(int)(surf->h*zoom/2), (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        }
        if (g->gameover) {
            surf = TTF_RenderText_Blended(g->font, "TIME'S UP!", (SDL_Color){255,200,0,255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ WIN_W/2-(int)(surf->w*zoom/2), WIN_H/2-(int)(surf->h*zoom/2), (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        }
    }

    SDL_RenderPresent(ren);
}

void game_free(Game *g) {
    for (int i=0;i<NUM_LVL;i++) {
        if (g->levels[i].bg) SDL_DestroyTexture(g->levels[i].bg);
        for (int j=0;j<3;j++)
            if (g->levels[i].pieces[j].tex) SDL_DestroyTexture(g->levels[i].pieces[j].tex);
    }
    if (g->background) SDL_DestroyTexture(g->background);
    if (g->snd_ok)     Mix_FreeChunk(g->snd_ok);
    if (g->snd_wrong)  Mix_FreeChunk(g->snd_wrong);
    if (g->music)      Mix_FreeMusic(g->music);
    if (g->font)       TTF_CloseFont(g->font);
    /* On detruit seulement la fenetre/renderer du puzzle, PAS SDL global */
    if (g->ren)        SDL_DestroyRenderer(g->ren);
    if (g->win)        SDL_DestroyWindow(g->win);
    Mix_HaltMusic();
}

/* ============================================================
   RunPuzzle : point d'entree appele depuis le menu du Quiz.
   Cree une fenetre puzzle, joue, puis revient au menu.
   ============================================================ */
void RunPuzzle(void) {
    Game puzzle;
    if (!game_init(&puzzle)) return;
    game_load(&puzzle);

    SDL_Event e;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)   running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;

            /* Quitter automatiquement apres "PUZZLE OK!" ou "TIME'S UP!" sur clic */
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if ((puzzle.solved && puzzle.cur == NUM_LVL-1) || puzzle.gameover)
                    running = 0;
            }
            game_event(&puzzle, &e);
        }
        game_update(&puzzle);
        game_draw(&puzzle);
    }

    game_free(&puzzle);
}
