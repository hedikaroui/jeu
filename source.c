#include "header.h"
int InitSDL(enigme *e)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        printf("Erreur IMG_Init : %s\n", IMG_GetError());
        return 0;
    }
    if (TTF_Init() != 0) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        return 0;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0) {
        printf("Erreur Mix_OpenAudio : %s\n", Mix_GetError());
        return 0;
    }
    return 1;
}
int Initialisation(enigme *e)
{
    int i;

    e->bg1              = NULL;
    e->bg2              = NULL;
    e->btnQuiz          = NULL;
    e->btnQuizHover     = NULL;
    e->btnPuzzle        = NULL;
    e->btnPuzzleHover   = NULL;
    e->btnA             = NULL;
    e->btnB             = NULL;
    e->btnC             = NULL;
    e->loser            = NULL;
    e->beep             = NULL;
    e->beep2            = NULL;
    e->laugh            = NULL;
    e->musique          = NULL;
    e->police           = NULL;

    e->etat         = 0;
    e->running      = 1;
    e->score        = 0;
    e->feedback     = 0;
    e->feedbackTimer= 0;
    e->tempsRestant = 15;
    e->enigmeFinie  = 0;
    e->repSurvol    = 0;
    e->nbQuestions  = 0;

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
int Load(enigme *e)
{
  
    e->police = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 20);
    if (!e->police)
        printf("Police non trouvee\n");    
    e->bg1 = IMG_LoadTexture(e->renderer, "bg1.png");
    e->bg2 = IMG_LoadTexture(e->renderer, "bg2.png");
    e->btnQuiz        = IMG_LoadTexture(e->renderer, "br.png");
    e->btnQuizHover   = IMG_LoadTexture(e->renderer, "bv.png");
    e->btnPuzzle      = IMG_LoadTexture(e->renderer, "brp.png");
    e->btnPuzzleHover = IMG_LoadTexture(e->renderer, "bvp.png");
    e->btnA = IMG_LoadTexture(e->renderer, "A.png");
    e->btnB = IMG_LoadTexture(e->renderer, "B.png");
    e->btnC = IMG_LoadTexture(e->renderer, "C.png");
    e->loser = IMG_LoadTexture(e->renderer, "loser.png");
    e->beep    = Mix_LoadWAV("beep.wav");
    e->beep2   = Mix_LoadWAV("beep2.wav");
    e->laugh   = Mix_LoadWAV("laugh.wav");
    e->musique = Mix_LoadMUS("musique.mp3");
    FILE *f = fopen("enigme.txt", "r");
    if (!f) {
        printf("Erreur : enigme.txt introuvable\n");
        return 0;
    }

    char ligne[300];
    int  idx = -1;

    while (fgets(ligne, sizeof(ligne), f))
    {
        int len = strlen(ligne);
        if (len > 0 && ligne[len-1] == '\n') ligne[len-1] = '\0';
        if (strlen(ligne) == 0) continue;

        if      (ligne[0]=='Q' && ligne[1]==':') { idx++; strcpy(e->questions[idx].question, ligne+2); }
        else if (ligne[0]=='A' && ligne[1]==':' && idx>=0) strcpy(e->questions[idx].repA, ligne+2);
        else if (ligne[0]=='B' && ligne[1]==':' && idx>=0) strcpy(e->questions[idx].repB, ligne+2);
        else if (ligne[0]=='C' && ligne[1]==':' && idx>=0) strcpy(e->questions[idx].repC, ligne+2);
        else if (ligne[0]=='R' && ligne[1]==':' && idx>=0)
        {
            if      (ligne[2]=='A') 
            	e->questions[idx].bonneReponse = 1;
            else if (ligne[2]=='B') 
            	e->questions[idx].bonneReponse = 2;
            else                    
            	e->questions[idx].bonneReponse = 3;
        }
    }
    fclose(f);

    e->nbQuestions = idx + 1;
    printf("%d questions chargees\n", e->nbQuestions);

    srand((unsigned int)time(NULL));
    NouvelleQuestion(e);

    return 1;
}
void NouvelleQuestion(enigme *e)
{
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
    e->feedbackTimer= 0;
    e->repSurvol    = 0;
}
void AfficherTexte(enigme *e, const char *txt, int x, int y, SDL_Color col, int taille)
{
    if (!e->police) return;

    TTF_SetFontSize(e->police, taille);

    SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(e->police, txt, col, 700);
    if (!surf) return;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(e->renderer, surf);
    SDL_FreeSurface(surf);
    if (!tex) return;

    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderCopy(e->renderer, tex, NULL, &r);
    SDL_DestroyTexture(tex);
}
void AfficherTimeline(enigme *e)
{
    int barreX      = 50;
    int barreY      = 50;
    int barreHauteur= 18;
    int barreMaxW   = 700;
    int barreW = (e->tempsRestant * barreMaxW) / 15;
    if (barreW < 0) barreW = 0;
    SDL_SetRenderDrawColor(e->renderer, 50, 50, 50, 200);
    SDL_Rect fond = {barreX, barreY, barreMaxW, barreHauteur};
    SDL_RenderFillRect(e->renderer, &fond);
    if (e->tempsRestant > 8)
        SDL_SetRenderDrawColor(e->renderer, 0, 200, 0, 255);   
    else if (e->tempsRestant > 4)
        SDL_SetRenderDrawColor(e->renderer, 255, 140, 0, 255);
    else
        SDL_SetRenderDrawColor(e->renderer, 220, 0, 0, 255);   

    SDL_Rect barre = {barreX, barreY, barreW, barreHauteur};
    SDL_RenderFillRect(e->renderer, &barre);
}
void Affichage(enigme *e)
{
    int mx, my;
    SDL_GetMouseState(&mx, &my);

    SDL_RenderClear(e->renderer);
    if (e->etat == 0)
    {
        if (e->bg1) SDL_RenderCopy(e->renderer, e->bg1, NULL, NULL);
        if (mx >= e->rectQuiz.x && mx <= e->rectQuiz.x + e->rectQuiz.w && my >= e->rectQuiz.y && my <= e->rectQuiz.y + e->rectQuiz.h)
        {
            if (e->btnQuizHover) SDL_RenderCopy(e->renderer, e->btnQuizHover, NULL, &e->rectQuiz);
        }
        else
        {
            if (e->btnQuiz) SDL_RenderCopy(e->renderer, e->btnQuiz, NULL, &e->rectQuiz);
        }
        if (mx >= e->rectPuzzle.x && mx <= e->rectPuzzle.x + e->rectPuzzle.w &&
            my >= e->rectPuzzle.y && my <= e->rectPuzzle.y + e->rectPuzzle.h)
        {
            if (e->btnPuzzleHover) SDL_RenderCopy(e->renderer, e->btnPuzzleHover, NULL, &e->rectPuzzle);
        }
        else
        {
            if (e->btnPuzzle) SDL_RenderCopy(e->renderer, e->btnPuzzle, NULL, &e->rectPuzzle);
        }
    }
    if (e->etat == 1)
    {
        SDL_Color blanc  = {255, 255, 255, 255};
        SDL_Color vert   = {  0, 220,   0, 255};
        SDL_Color rouge  = {220,   0,   0, 255};
        SDL_Color noir  = {0, 0,   0, 255};
        if (e->enigmeFinie == 0)
        {
            if (e->bg2) SDL_RenderCopy(e->renderer, e->bg2, NULL, NULL);
            AfficherTimeline(e);
            char scoreTxt[30];
            sprintf(scoreTxt, "Score : %d", e->score);
            AfficherTexte(e, scoreTxt, 20, 20, blanc, 30);
            AfficherTexte(e, e->actuelle.question, 50, 80, noir, 28);
            if (e->btnA) SDL_RenderCopy(e->renderer, e->btnA, NULL, &e->rectA);
            if (e->btnB) SDL_RenderCopy(e->renderer, e->btnB, NULL, &e->rectB);
            if (e->btnC) SDL_RenderCopy(e->renderer, e->btnC, NULL, &e->rectC);
            if (e->repSurvol == 1) AfficherTexte(e, e->actuelle.repA, 80,  495, noir, 24);
            if (e->repSurvol == 2) AfficherTexte(e, e->actuelle.repB, 310, 495, noir, 24);
            if (e->repSurvol == 3) AfficherTexte(e, e->actuelle.repC, 560, 495, noir, 24);
        }
        if (e->enigmeFinie != 0)
        {
      
            if (e->bg1) SDL_RenderCopy(e->renderer, e->bg1, NULL, NULL);
            char scoreTxt[30];
            sprintf(scoreTxt, "Score : %d", e->score);
            AfficherTexte(e, scoreTxt, 20, 20, blanc, 30);
            if (e->feedback == 1)
            {
                AfficherTexte(e, "WELL DONE ", 90, 200, vert, 100);
                AfficherTexte(e, "Cliquez pour continuer...", 220, 300, blanc, 28);
            }
            if (e->feedback == -1)
            {
                AfficherTexte(e, "WRONG !", 240, 160, rouge, 100);

                char bonneRep[200];
                if      (e->actuelle.bonneReponse == 1)
                    sprintf(bonneRep, "Bonne reponse : %s", e->actuelle.repA);
                else if (e->actuelle.bonneReponse == 2)
                    sprintf(bonneRep, "Bonne reponse : %s", e->actuelle.repB);
                else
                    sprintf(bonneRep, "Bonne reponse : %s", e->actuelle.repC);

                AfficherTexte(e, bonneRep, 120, 250, noir, 50);

                SDL_Rect rLoser = {330, 330, 130, 130};
                if (e->loser) SDL_RenderCopy(e->renderer, e->loser, NULL, &rLoser);

                AfficherTexte(e, "Cliquez pour continuer...", 220, 490, blanc, 28);
            }
            if (e->enigmeFinie == 2 && e->feedback == 0)
            {
                AfficherTexte(e, "Temps ecoule !", 210, 200, rouge, 50);
                AfficherTexte(e, "Cliquez pour continuer...", 220, 300, blanc, 28);
            }
        }
    }

    SDL_RenderPresent(e->renderer);
}
void GererEvenement(enigme *e, int *running)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT) *running = 0;

        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                if (e->etat == 1)
                {
                    Mix_HaltMusic();
                    e->etat     = 0;
                    e->score    = 0;
                    e->feedback = 0;
                }
                else *running = 0;
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            int mx = event.button.x;
            int my = event.button.y;
            if (e->etat == 0)
            {
                if (mx >= e->rectQuiz.x && mx <= e->rectQuiz.x + e->rectQuiz.w &&
                    my >= e->rectQuiz.y && my <= e->rectQuiz.y + e->rectQuiz.h)
                {
                    if (e->beep) Mix_PlayChannel(-1, e->beep, 0);
                    e->etat = 1;
                    NouvelleQuestion(e);
                    if (e->musique) Mix_PlayMusic(e->musique, -1);
                }
                if (mx >= e->rectPuzzle.x && mx <= e->rectPuzzle.x + e->rectPuzzle.w &&
                    my >= e->rectPuzzle.y && my <= e->rectPuzzle.y + e->rectPuzzle.h)
                {
                    if (e->beep) Mix_PlayChannel(-1, e->beep, 0);
                }
            }
            if (e->etat == 1)
            {
                if (e->enigmeFinie != 0)
                {
                    Mix_HaltMusic();
                    e->etat     = 0;
                    e->score    = 0;
                    e->feedback = 0;
                    e->enigmeFinie = 0;
                }
                else
                {
                    int rep = 0;

                    if (mx >= e->rectA.x && mx <= e->rectA.x + e->rectA.w &&
                        my >= e->rectA.y && my <= e->rectA.y + e->rectA.h) rep = 1;

                    if (mx >= e->rectB.x && mx <= e->rectB.x + e->rectB.w &&
                        my >= e->rectB.y && my <= e->rectB.y + e->rectB.h) rep = 2;

                    if (mx >= e->rectC.x && mx <= e->rectC.x + e->rectC.w &&
                        my >= e->rectC.y && my <= e->rectC.y + e->rectC.h) rep = 3;

                    if (rep != 0)
                    {
                        if (e->beep) Mix_PlayChannel(-1, e->beep, 0);

                        if (rep == e->actuelle.bonneReponse)
                        {
                            e->feedback = 1;
                            e->score   += 20;
                            if (e->beep2) Mix_PlayChannel(-1, e->beep2, 0);
                        }
                        else
                        {
                            e->feedback = -1;
                            e->score    = 0;
                            if (e->laugh) Mix_PlayChannel(-1, e->laugh, 0);
                        }

                        e->enigmeFinie  = 1;
                        e->feedbackTimer= SDL_GetTicks();
                    }
                }
            }
        }

        /* Survol boutons reponses */
        if (event.type == SDL_MOUSEMOTION && e->etat == 1)
        {
            int mx = event.motion.x;
            int my = event.motion.y;
            e->repSurvol = 0;

            if (mx >= e->rectA.x && mx <= e->rectA.x + e->rectA.w &&
                my >= e->rectA.y && my <= e->rectA.y + e->rectA.h) e->repSurvol = 1;

            if (mx >= e->rectB.x && mx <= e->rectB.x + e->rectB.w &&
                my >= e->rectB.y && my <= e->rectB.y + e->rectB.h) e->repSurvol = 2;

            if (mx >= e->rectC.x && mx <= e->rectC.x + e->rectC.w &&
                my >= e->rectC.y && my <= e->rectC.y + e->rectC.h) e->repSurvol = 3;
        }
    }
}
void MiseAJour(enigme *e)
{
    if (e->etat != 1) return;
    if (e->enigmeFinie != 0) return;

    Uint32 maintenant = SDL_GetTicks();
    int ecoule = (int)((maintenant - e->tempsDepart) / 1000);
    e->tempsRestant = 15 - ecoule;
    if (e->tempsRestant <= 0)
    {
        e->tempsRestant = 0;
        e->enigmeFinie  = 2;   
        e->score        = 0;
        if (e->laugh) Mix_PlayChannel(-1, e->laugh, 0);
    }
}
void Liberation(enigme *e)
{
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

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(e->renderer);
    SDL_DestroyWindow(e->window);
    SDL_Quit();
}
