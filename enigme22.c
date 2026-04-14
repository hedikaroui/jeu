#include "enigme22.h"

/* ═══════════════════════════════════════════════════════════════════
   PUZZLE  (adapte de puzzle_source.c – branche saidane22)
═══════════════════════════════════════════════════════════════════ */

int E22_inside(SDL_Rect *r, int x, int y) {
    return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h;
}

int E22_overlap(SDL_Rect *a, SDL_Rect *b) {
    return !(a->x + a->w < b->x || b->x + b->w < a->x ||
             a->y + a->h < b->y || b->y + b->h < a->y);
}

void E22_reset_pieces(E22_Level *lv) {
    int ord[3] = {0, 1, 2};
    for (int i = 2; i > 0; i--) {
        int j = rand() % (i + 1), tmp = ord[i];
        ord[i] = ord[j]; ord[j] = tmp;
    }
    for (int i = 0; i < 3; i++) {
        lv->pieces[ord[i]].home = (SDL_Rect){ E22_WIN_W - 165, 80 + i * 160, 140, 140 };
        lv->pieces[ord[i]].dst  = lv->pieces[ord[i]].home;
        lv->pieces[ord[i]].dragging = 0;
    }
}

int E22_game_init(E22_PuzzleGame *g) {
    srand((unsigned)time(NULL));
    *g = (E22_PuzzleGame){0};
    /* SDL est deja initialise – on cree juste une nouvelle fenetre pour le puzzle */
    g->win = SDL_CreateWindow("Puzzle",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                E22_WIN_W, E22_WIN_H, SDL_WINDOW_SHOWN);
    g->ren = SDL_CreateRenderer(g->win, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    g->font      = TTF_OpenFont("arial.ttf", 52);
    if (!g->font) g->font = TTF_OpenFont("font.ttf", 52);
    g->music     = Mix_LoadMUS("musique.mp3");
    g->snd_ok    = Mix_LoadWAV("beep2.wav");
    g->snd_wrong = Mix_LoadWAV("laugh.wav");
    if (g->music) { Mix_VolumeMusic(64); Mix_PlayMusic(g->music, -1); }
    return g->win && g->ren;
}

void E22_game_load(E22_PuzzleGame *g) {
    int pw = E22_WIN_W - 180, ph = E22_WIN_H - 50;
    g->background = IMG_LoadTexture(g->ren, "bg2.png");

    E22_Level *l0 = &g->levels[0];
    l0->bg = IMG_LoadTexture(g->ren, "picture1.png");
    l0->hole = (SDL_Rect){
        (int)(687.f / 979  * pw), 50 + (int)(114.f / 645 * ph),
        (int)( 98.f / 979  * pw),      (int)( 99.f / 645 * ph)
    };
    l0->pieces[0] = (E22_Piece){ IMG_LoadTexture(g->ren, "correct1.png"), {0},{0}, 1 };
    l0->pieces[1] = (E22_Piece){ IMG_LoadTexture(g->ren, "wrong1.png"),   {0},{0}, 0 };
    l0->pieces[2] = (E22_Piece){ IMG_LoadTexture(g->ren, "wrong11.png"),  {0},{0}, 0 };
    E22_reset_pieces(l0);

    E22_Level *l1 = &g->levels[1];
    l1->bg = IMG_LoadTexture(g->ren, "picture2.png");
    l1->hole = (SDL_Rect){
        (int)(524.f / 1196 * pw), 50 + (int)(371.f / 896 * ph),
        (int)(151.f / 1196 * pw),      (int)(152.f / 896 * ph)
    };
    l1->pieces[0] = (E22_Piece){ IMG_LoadTexture(g->ren, "correct2.png"), {0},{0}, 1 };
    l1->pieces[1] = (E22_Piece){ IMG_LoadTexture(g->ren, "wrong2.png"),   {0},{0}, 0 };
    l1->pieces[2] = (E22_Piece){ IMG_LoadTexture(g->ren, "wrong22.png"),  {0},{0}, 0 };
    E22_reset_pieces(l1);

    g->cur = 0;
    g->t0  = SDL_GetTicks();
}

void E22_game_event(E22_PuzzleGame *g, SDL_Event *e) {
    if (g->solved || g->gameover) return;
    E22_Level *lv = &g->levels[g->cur];

    if (e->type == SDL_MOUSEBUTTONDOWN)
        for (int i = 0; i < 3; i++)
            if (E22_inside(&lv->pieces[i].dst, e->button.x, e->button.y)) {
                lv->pieces[i].dragging = 1;
                lv->pieces[i].ox = e->button.x - lv->pieces[i].dst.x;
                lv->pieces[i].oy = e->button.y - lv->pieces[i].dst.y;
            }

    if (e->type == SDL_MOUSEMOTION)
        for (int i = 0; i < 3; i++)
            if (lv->pieces[i].dragging) {
                lv->pieces[i].dst.x = e->motion.x - lv->pieces[i].ox;
                lv->pieces[i].dst.y = e->motion.y - lv->pieces[i].oy;
            }

    if (e->type == SDL_MOUSEBUTTONUP)
        for (int i = 0; i < 3; i++) {
            E22_Piece *p = &lv->pieces[i];
            if (!p->dragging) continue;
            p->dragging = 0;
            if (E22_overlap(&p->dst, &lv->hole)) {
                if (p->correct) {
                    p->dst       = lv->hole;
                    g->solved    = 1;
                    g->solved_at = SDL_GetTicks();
                    if (g->cur == E22_NUM_LVL - 1 && g->snd_ok)
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

void E22_game_update(E22_PuzzleGame *g) {
    if (g->wrong_flash > 0) g->wrong_flash--;
    if (g->gameover) return;
    if (g->solved) {
        if (SDL_GetTicks() - g->solved_at > 1500) {
            if (g->cur + 1 < E22_NUM_LVL) {
                g->cur++;
                g->t0     = SDL_GetTicks();
                g->solved = 0;
                E22_reset_pieces(&g->levels[g->cur]);
            }
        }
        return;
    }
    if ((int)(SDL_GetTicks() - g->t0) >= E22_TIMER_SEC * 1000)
        g->gameover = 1;
}

void E22_game_draw(E22_PuzzleGame *g) {
    SDL_Renderer *ren = g->ren;
    E22_Level *lv = &g->levels[g->cur];

    SDL_Rect full = {0, 0, E22_WIN_W, E22_WIN_H};
    SDL_RenderCopy(ren, g->background, NULL, &full);

    SDL_Rect bg_dst = {0, 50, E22_WIN_W - 180, E22_WIN_H - 50};
    SDL_RenderCopy(ren, lv->bg, NULL, &bg_dst);

    if (!g->solved) {
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderFillRect(ren, &lv->hole);
        SDL_SetRenderDrawColor(ren, 255, 210, 0, 255);
        SDL_RenderDrawRect(ren, &lv->hole);
    }

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 10, 10, 20, 200);
    SDL_Rect panel = {E22_WIN_W - 175, 45, 175, E22_WIN_H - 45};
    SDL_RenderFillRect(ren, &panel);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

    for (int i = 0; i < 3; i++) {
        E22_Piece *p = &lv->pieces[i];
        if (p->dragging) {
            SDL_SetRenderDrawColor(ren, 255, 210, 0, 255);
            SDL_Rect b = {p->dst.x - 3, p->dst.y - 3, p->dst.w + 6, p->dst.h + 6};
            SDL_RenderFillRect(ren, &b);
        }
        SDL_RenderCopy(ren, p->tex, NULL, &p->dst);
    }

    float ratio = 1.0f - (float)(SDL_GetTicks() - g->t0) / (E22_TIMER_SEC * 1000.f);
    if (ratio < 0) ratio = 0;

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 150);
    SDL_Rect bgbar = {0, 0, E22_WIN_W, 50};
    SDL_RenderFillRect(ren, &bgbar);

    int max_w = E22_WIN_W - 40, h = 16, x = 20, y = 25;
    int w = (int)(max_w * ratio);
    SDL_SetRenderDrawColor(ren,
        255, (Uint8)(220 * ratio), (Uint8)(150 * ratio), 255);
    SDL_Rect candle = {x, y - h / 2, w, h};
    SDL_RenderFillRect(ren, &candle);

    if (ratio > 0) {
        SDL_SetRenderDrawColor(ren, 255, 140, 0, 255);
        SDL_Rect flame = {x + w - 2, y - 8 - rand() % 3, 5, 8};
        SDL_RenderFillRect(ren, &flame);
    }

    if (g->font) {
        SDL_Surface *surf;
        SDL_Texture *tex;
        SDL_Rect dst;
        double angle = SDL_GetTicks() / 10.0;
        double zoom  = 1.0 + 0.15 * sin(SDL_GetTicks() / 200.0);

        if (g->wrong_flash > 0) {
            surf = TTF_RenderText_Blended(g->font, "WRONG!", (SDL_Color){220, 30, 30, 255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ E22_WIN_W/2 - (int)(surf->w*zoom/2), E22_WIN_H/2 - (int)(surf->h*zoom/2),
                               (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        }
        if (g->solved && g->cur == E22_NUM_LVL - 1) {
            surf = TTF_RenderText_Blended(g->font, "PUZZLE OK!", (SDL_Color){50, 220, 80, 255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ E22_WIN_W/2 - (int)(surf->w*zoom/2), E22_WIN_H/2 - (int)(surf->h*zoom/2),
                               (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        } else if (g->solved) {
            surf = TTF_RenderText_Blended(g->font, "NEXT!", (SDL_Color){50, 180, 255, 255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ E22_WIN_W/2 - (int)(surf->w*zoom/2), E22_WIN_H/2 - (int)(surf->h*zoom/2),
                               (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        }
        if (g->gameover) {
            surf = TTF_RenderText_Blended(g->font, "TIME'S UP!", (SDL_Color){255, 200, 0, 255});
            tex  = SDL_CreateTextureFromSurface(ren, surf);
            dst  = (SDL_Rect){ E22_WIN_W/2 - (int)(surf->w*zoom/2), E22_WIN_H/2 - (int)(surf->h*zoom/2),
                               (int)(surf->w*zoom), (int)(surf->h*zoom) };
            SDL_FreeSurface(surf);
            SDL_RenderCopyEx(ren, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
            SDL_DestroyTexture(tex);
        }
    }

    SDL_RenderPresent(ren);
}

void E22_game_free(E22_PuzzleGame *g) {
    for (int i = 0; i < E22_NUM_LVL; i++) {
        if (g->levels[i].bg) SDL_DestroyTexture(g->levels[i].bg);
        for (int j = 0; j < 3; j++)
            if (g->levels[i].pieces[j].tex) SDL_DestroyTexture(g->levels[i].pieces[j].tex);
    }
    if (g->background) SDL_DestroyTexture(g->background);
    if (g->snd_ok)     Mix_FreeChunk(g->snd_ok);
    if (g->snd_wrong)  Mix_FreeChunk(g->snd_wrong);
    if (g->music)      Mix_FreeMusic(g->music);
    if (g->font)       TTF_CloseFont(g->font);
    Mix_HaltMusic();
    if (g->ren) SDL_DestroyRenderer(g->ren);
    if (g->win) SDL_DestroyWindow(g->win);
}

/* Lance le puzzle dans sa propre fenetre (comportement original) */
void E22_RunPuzzle(void) {
    E22_PuzzleGame puzzle;
    if (!E22_game_init(&puzzle)) return;
    E22_game_load(&puzzle);

    SDL_Event e;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
            if (e.type == SDL_MOUSEBUTTONDOWN)
                if ((puzzle.solved && puzzle.cur == E22_NUM_LVL - 1) || puzzle.gameover)
                    running = 0;
            E22_game_event(&puzzle, &e);
        }
        E22_game_update(&puzzle);
        E22_game_draw(&puzzle);
    }

    E22_game_free(&puzzle);
}

/* ═══════════════════════════════════════════════════════════════════
   QUIZ  (adapte de source.c – branche saidane22)
═══════════════════════════════════════════════════════════════════ */

int E22_Initialisation(enigme22 *e) {
    int i;

    e->bg1 = e->bg2 = NULL;
    e->btnQuiz = e->btnQuizHover = NULL;
    e->btnPuzzle = e->btnPuzzleHover = NULL;
    e->btnA = e->btnB = e->btnC = NULL;
    e->loser = NULL;
    e->beep = e->beep2 = e->laugh = NULL;
    e->musique = NULL;
    e->police = NULL;

    e->etat          = 0;
    e->score         = 0;
    e->feedback      = 0;
    e->feedbackTimer = 0;
    e->tempsRestant  = 15;
    e->enigmeFinie   = 0;
    e->repSurvol     = 0;
    e->nbQuestions   = 0;

    for (i = 0; i < 50; i++)
        e->dejaPosee[i] = 0;

    e->rectQuiz.x  = 200; e->rectQuiz.y  = 320;
    e->rectQuiz.w  = 160; e->rectQuiz.h  = 60;

    e->rectPuzzle.x = 440; e->rectPuzzle.y = 320;
    e->rectPuzzle.w = 160; e->rectPuzzle.h = 60;

    e->rectA.x = 80;  e->rectA.y = 430; e->rectA.w = 130; e->rectA.h = 55;
    e->rectB.x = 335; e->rectB.y = 430; e->rectB.w = 130; e->rectB.h = 55;
    e->rectC.x = 590; e->rectC.y = 430; e->rectC.w = 130; e->rectC.h = 55;

    return 1;
}

int E22_Load(enigme22 *e, SDL_Renderer *ren) {
    /* Police : essai dans l'ordre */
    e->police = TTF_OpenFont("arial.ttf", 20);
    if (!e->police)
        e->police = TTF_OpenFont("font.ttf", 20);
    if (!e->police)
        e->police = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20);
    if (!e->police)
        printf("[E22] Police non trouvee\n");

    e->bg1 = IMG_LoadTexture(ren, "bg1.png");
    e->bg2 = IMG_LoadTexture(ren, "bg2.png");

    e->btnQuiz        = IMG_LoadTexture(ren, "br.png");
    e->btnQuizHover   = IMG_LoadTexture(ren, "bv.png");
    e->btnPuzzle      = IMG_LoadTexture(ren, "brp.png");
    e->btnPuzzleHover = IMG_LoadTexture(ren, "bvp.png");

    e->btnA = IMG_LoadTexture(ren, "A.png");
    e->btnB = IMG_LoadTexture(ren, "B.png");
    e->btnC = IMG_LoadTexture(ren, "C.png");

    e->loser = IMG_LoadTexture(ren, "loser.png");

    e->beep    = Mix_LoadWAV("beep.wav");
    e->beep2   = Mix_LoadWAV("beep2.wav");
    e->laugh   = Mix_LoadWAV("laugh.wav");
    e->musique = Mix_LoadMUS("musique.mp3");

    FILE *f = fopen("enigme.txt", "r");
    if (!f) {
        printf("[E22] enigme.txt introuvable\n");
        return 0;
    }

    char ligne[300];
    int  idx = -1;

    while (fgets(ligne, sizeof(ligne), f)) {
        int len = strlen(ligne);
        if (len > 0 && ligne[len - 1] == '\n') ligne[len - 1] = '\0';
        if (strlen(ligne) == 0) continue;

        if      (ligne[0] == 'Q' && ligne[1] == ':') {
            idx++;
            strcpy(e->questions[idx].question, ligne + 2);
        } else if (ligne[0] == 'A' && ligne[1] == ':' && idx >= 0) {
            strcpy(e->questions[idx].repA, ligne + 2);
        } else if (ligne[0] == 'B' && ligne[1] == ':' && idx >= 0) {
            strcpy(e->questions[idx].repB, ligne + 2);
        } else if (ligne[0] == 'C' && ligne[1] == ':' && idx >= 0) {
            strcpy(e->questions[idx].repC, ligne + 2);
        } else if (ligne[0] == 'R' && ligne[1] == ':' && idx >= 0) {
            if      (ligne[2] == 'A') e->questions[idx].bonneReponse = 1;
            else if (ligne[2] == 'B') e->questions[idx].bonneReponse = 2;
            else                      e->questions[idx].bonneReponse = 3;
        }
    }
    fclose(f);

    e->nbQuestions = idx + 1;
    printf("[E22] %d questions chargees\n", e->nbQuestions);

    srand((unsigned int)time(NULL));
    E22_NouvelleQuestion(e);

    return 1;
}

void E22_NouvelleQuestion(enigme22 *e) {
    int i, reste = 0;

    for (i = 0; i < e->nbQuestions; i++)
        if (e->dejaPosee[i] == 0) reste++;

    if (reste == 0)
        for (i = 0; i < e->nbQuestions; i++)
            e->dejaPosee[i] = 0;

    int idx;
    do { idx = rand() % e->nbQuestions; }
    while (e->dejaPosee[idx] == 1);

    e->dejaPosee[idx] = 1;
    e->actuelle        = e->questions[idx];

    e->tempsDepart  = SDL_GetTicks();
    e->tempsRestant = 15;
    e->enigmeFinie  = 0;
    e->feedback     = 0;
    e->feedbackTimer = 0;
    e->repSurvol    = 0;
}

void E22_AfficherTexte(SDL_Renderer *ren, TTF_Font *police, const char *txt,
                       int x, int y, SDL_Color col, int taille) {
    if (!police) return;

    TTF_SetFontSize(police, taille);

    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(police, txt, col, 700);
    if (!surf) return;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;

    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderCopy(ren, tex, NULL, &r);
    SDL_DestroyTexture(tex);
}

void E22_AfficherTimeline(enigme22 *e, SDL_Renderer *ren) {
    int barreX       = 50;
    int barreY       = 50;
    int barreHauteur = 18;
    int barreMaxW    = 700;

    int barreW = (e->tempsRestant * barreMaxW) / 15;
    if (barreW < 0) barreW = 0;

    SDL_SetRenderDrawColor(ren, 50, 50, 50, 200);
    SDL_Rect fond = {barreX, barreY, barreMaxW, barreHauteur};
    SDL_RenderFillRect(ren, &fond);

    if (e->tempsRestant > 8)
        SDL_SetRenderDrawColor(ren, 0, 200, 0, 255);
    else if (e->tempsRestant > 4)
        SDL_SetRenderDrawColor(ren, 255, 140, 0, 255);
    else
        SDL_SetRenderDrawColor(ren, 220, 0, 0, 255);

    SDL_Rect barre = {barreX, barreY, barreW, barreHauteur};
    SDL_RenderFillRect(ren, &barre);
}

/* ──────────────────────────────────────────────────────────────
   E22_Affichage : dessine le popup en overlay sur le renderer principal.
   Doit etre appele apres le rendu du jeu, avant SDL_RenderPresent.
────────────────────────────────────────────────────────────── */
void E22_Affichage(enigme22 *e, SDL_Renderer *ren) {
    /* Coordonnees souris en espace popup */
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    mx -= E22_POP_X;
    my -= E22_POP_Y;

    /* 1. Fond semi-transparent sur tout l'ecran */
    SDL_RenderSetViewport(ren, NULL);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 180);
    SDL_Rect full = {0, 0, 1400, 720};
    SDL_RenderFillRect(ren, &full);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);

    /* 2. Viewport restreint a la zone popup */
    SDL_Rect vp = {E22_POP_X, E22_POP_Y, E22_POP_W, E22_POP_H};
    SDL_RenderSetViewport(ren, &vp);

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color vert  = {  0, 220,   0, 255};
    SDL_Color rouge = {220,   0,   0, 255};
    SDL_Color noir  = {  0,   0,   0, 255};

    /* ──── MENU ──── */
    if (e->etat == 0) {
        if (e->bg1) SDL_RenderCopy(ren, e->bg1, NULL, NULL);

        if (mx >= e->rectQuiz.x && mx <= e->rectQuiz.x + e->rectQuiz.w &&
            my >= e->rectQuiz.y && my <= e->rectQuiz.y + e->rectQuiz.h)
        {
            if (e->btnQuizHover) SDL_RenderCopy(ren, e->btnQuizHover, NULL, &e->rectQuiz);
        } else {
            if (e->btnQuiz) SDL_RenderCopy(ren, e->btnQuiz, NULL, &e->rectQuiz);
        }

        if (mx >= e->rectPuzzle.x && mx <= e->rectPuzzle.x + e->rectPuzzle.w &&
            my >= e->rectPuzzle.y && my <= e->rectPuzzle.y + e->rectPuzzle.h)
        {
            if (e->btnPuzzleHover) SDL_RenderCopy(ren, e->btnPuzzleHover, NULL, &e->rectPuzzle);
        } else {
            if (e->btnPuzzle) SDL_RenderCopy(ren, e->btnPuzzle, NULL, &e->rectPuzzle);
        }

        E22_AfficherTexte(ren, e->police, "ECHAP pour fermer", 270, 565, blanc, 18);
    }

    /* ──── QUIZ ──── */
    if (e->etat == 1) {
        if (e->enigmeFinie == 0) {
            if (e->bg2) SDL_RenderCopy(ren, e->bg2, NULL, NULL);

            E22_AfficherTimeline(e, ren);

            char scoreTxt[30];
            sprintf(scoreTxt, "Score : %d", e->score);
            E22_AfficherTexte(ren, e->police, scoreTxt, 20, 20, blanc, 30);
            E22_AfficherTexte(ren, e->police, e->actuelle.question, 50, 80, noir, 28);

            if (e->btnA) SDL_RenderCopy(ren, e->btnA, NULL, &e->rectA);
            if (e->btnB) SDL_RenderCopy(ren, e->btnB, NULL, &e->rectB);
            if (e->btnC) SDL_RenderCopy(ren, e->btnC, NULL, &e->rectC);

            if (e->repSurvol == 1) E22_AfficherTexte(ren, e->police, e->actuelle.repA,  80, 495, noir, 24);
            if (e->repSurvol == 2) E22_AfficherTexte(ren, e->police, e->actuelle.repB, 310, 495, noir, 24);
            if (e->repSurvol == 3) E22_AfficherTexte(ren, e->police, e->actuelle.repC, 560, 495, noir, 24);
        } else {
            if (e->bg1) SDL_RenderCopy(ren, e->bg1, NULL, NULL);

            char scoreTxt[30];
            sprintf(scoreTxt, "Score : %d", e->score);
            E22_AfficherTexte(ren, e->police, scoreTxt, 20, 20, blanc, 30);

            if (e->feedback == 1) {
                E22_AfficherTexte(ren, e->police, "WELL DONE !", 200, 200, vert, 80);
                E22_AfficherTexte(ren, e->police, "Cliquez pour continuer...", 220, 310, blanc, 28);
            }

            if (e->feedback == -1) {
                E22_AfficherTexte(ren, e->police, "WRONG !", 240, 160, rouge, 80);

                char bonneRep[200];
                if      (e->actuelle.bonneReponse == 1)
                    sprintf(bonneRep, "Bonne reponse : %s", e->actuelle.repA);
                else if (e->actuelle.bonneReponse == 2)
                    sprintf(bonneRep, "Bonne reponse : %s", e->actuelle.repB);
                else
                    sprintf(bonneRep, "Bonne reponse : %s", e->actuelle.repC);

                E22_AfficherTexte(ren, e->police, bonneRep, 60, 260, noir, 30);

                SDL_Rect rLoser = {330, 340, 130, 130};
                if (e->loser) SDL_RenderCopy(ren, e->loser, NULL, &rLoser);

                E22_AfficherTexte(ren, e->police, "Cliquez pour continuer...", 220, 490, blanc, 28);
            }

            if (e->enigmeFinie == 2 && e->feedback == 0) {
                E22_AfficherTexte(ren, e->police, "Temps ecoule !", 210, 200, rouge, 50);
                E22_AfficherTexte(ren, e->police, "Cliquez pour continuer...", 220, 300, blanc, 28);
            }
        }
    }

    /* 3. Retablir le viewport global */
    SDL_RenderSetViewport(ren, NULL);
}

/* ──────────────────────────────────────────────────────────────
   E22_GererEvenement : traite un evenement SDL deja recupere
   dans la boucle principale.
   Positionne *fermer=1 pour demander la fermeture du popup.
────────────────────────────────────────────────────────────── */
void E22_GererEvenement(enigme22 *e, SDL_Event *ev, int *fermer) {
    if (ev->type == SDL_KEYDOWN) {
        if (ev->key.keysym.sym == SDLK_ESCAPE) {
            if (e->etat == 1) {
                Mix_HaltMusic();
                e->etat     = 0;
                e->score    = 0;
                e->feedback = 0;
            } else {
                *fermer = 1;
            }
        }
    }

    if (ev->type == SDL_MOUSEBUTTONDOWN && ev->button.button == SDL_BUTTON_LEFT) {
        /* Coordonnees relatives au viewport popup */
        int mx = ev->button.x - E22_POP_X;
        int my = ev->button.y - E22_POP_Y;

        /* ===== MENU ===== */
        if (e->etat == 0) {
            if (mx >= e->rectQuiz.x && mx <= e->rectQuiz.x + e->rectQuiz.w &&
                my >= e->rectQuiz.y && my <= e->rectQuiz.y + e->rectQuiz.h)
            {
                if (e->beep) Mix_PlayChannel(-1, e->beep, 0);
                e->etat = 1;
                E22_NouvelleQuestion(e);
                if (e->musique) Mix_PlayMusic(e->musique, -1);
            }

            if (mx >= e->rectPuzzle.x && mx <= e->rectPuzzle.x + e->rectPuzzle.w &&
                my >= e->rectPuzzle.y && my <= e->rectPuzzle.y + e->rectPuzzle.h)
            {
                if (e->beep) Mix_PlayChannel(-1, e->beep, 0);
                E22_RunPuzzle();
            }
        }

        /* ===== QUIZ ===== */
        if (e->etat == 1) {
            if (e->enigmeFinie != 0) {
                Mix_HaltMusic();
                e->etat        = 0;
                e->score       = 0;
                e->feedback    = 0;
                e->enigmeFinie = 0;
                E22_NouvelleQuestion(e);
            } else {
                int rep = 0;

                if (mx >= e->rectA.x && mx <= e->rectA.x + e->rectA.w &&
                    my >= e->rectA.y && my <= e->rectA.y + e->rectA.h) rep = 1;
                if (mx >= e->rectB.x && mx <= e->rectB.x + e->rectB.w &&
                    my >= e->rectB.y && my <= e->rectB.y + e->rectB.h) rep = 2;
                if (mx >= e->rectC.x && mx <= e->rectC.x + e->rectC.w &&
                    my >= e->rectC.y && my <= e->rectC.y + e->rectC.h) rep = 3;

                if (rep != 0) {
                    if (e->beep) Mix_PlayChannel(-1, e->beep, 0);

                    if (rep == e->actuelle.bonneReponse) {
                        e->feedback = 1;
                        e->score   += 20;
                        if (e->beep2) Mix_PlayChannel(-1, e->beep2, 0);
                    } else {
                        e->feedback = -1;
                        e->score    = 0;
                        if (e->laugh) Mix_PlayChannel(-1, e->laugh, 0);
                    }

                    e->enigmeFinie   = 1;
                    e->feedbackTimer = SDL_GetTicks();
                }
            }
        }
    }

    if (ev->type == SDL_MOUSEMOTION && e->etat == 1) {
        int mx = ev->motion.x - E22_POP_X;
        int my = ev->motion.y - E22_POP_Y;
        e->repSurvol = 0;

        if (mx >= e->rectA.x && mx <= e->rectA.x + e->rectA.w &&
            my >= e->rectA.y && my <= e->rectA.y + e->rectA.h) e->repSurvol = 1;
        if (mx >= e->rectB.x && mx <= e->rectB.x + e->rectB.w &&
            my >= e->rectB.y && my <= e->rectB.y + e->rectB.h) e->repSurvol = 2;
        if (mx >= e->rectC.x && mx <= e->rectC.x + e->rectC.w &&
            my >= e->rectC.y && my <= e->rectC.y + e->rectC.h) e->repSurvol = 3;
    }
}

void E22_MiseAJour(enigme22 *e) {
    if (e->etat != 1) return;
    if (e->enigmeFinie != 0) return;

    Uint32 maintenant = SDL_GetTicks();
    int ecoule = (int)((maintenant - e->tempsDepart) / 1000);
    e->tempsRestant = 15 - ecoule;

    if (e->tempsRestant <= 0) {
        e->tempsRestant = 0;
        e->enigmeFinie  = 2;
        e->score        = 0;
        if (e->laugh) Mix_PlayChannel(-1, e->laugh, 0);
    }
}

void E22_Liberation(enigme22 *e) {
    SDL_DestroyTexture(e->bg1);
    SDL_DestroyTexture(e->bg2);
    SDL_DestroyTexture(e->btnQuiz);
    SDL_DestroyTexture(e->btnQuizHover);
    SDL_DestroyTexture(e->btnPuzzle);
    SDL_DestroyTexture(e->btnPuzzleHover);
    SDL_DestroyTexture(e->btnA);
    SDL_DestroyTexture(e->btnB);
    SDL_DestroyTexture(e->btnC);
    SDL_DestroyTexture(e->loser);

    Mix_FreeChunk(e->beep);
    Mix_FreeChunk(e->beep2);
    Mix_FreeChunk(e->laugh);
    Mix_FreeMusic(e->musique);

    if (e->police) TTF_CloseFont(e->police);
}
