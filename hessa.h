#include "header.h"

int main()
{
//----------------declaration--------------------------
    Menu menu;
    SDL_Event event;
    int running;
    int interface=0; //0 mp , 1 mo, 2 mms, 3 jeu
//----------------INIT SDL--------------------------

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    window = SDL_CreateWindow("Menu Principal",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);

    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED);
        
        
//----------------Init Ressources--------------------------
    Initialisation(&menu,renderer,&running);

    while(running)  
    {
//----------------Display--------------------------
        if ( interface ==0) 
              AffichageMenuPrincipal(&menu,renderer);
        if ( interface ==1) 
              AffichageMenuOption(renderer,&interface); //(&menuOption,renderer)
        if ( interface==2) 
             AffichageMenuScore(renderer,&interface); //(&menuScore,renderer)
        

    while(SDL_PollEvent(&event))
    {
     if(event.type == SDL_QUIT)
           running = 0;
  // ----------- ESC POUR RETOUR MENU PRINCIPAL -----------
    if(event.type == SDL_KEYDOWN)
    {
        if(event.key.keysym.sym == SDLK_ESCAPE)
        {
            if(interface == 1 || interface == 2)
            {
                interface = 0;  // Retour menu principal
            }
        }
    }
 //----------------Input--------------------------      
        if ( interface ==0)
              LectureEntreeMenuPrincipal(&menu, &event, &running);
      /*if ( interface ==1) 
              LectureEntreeMenuOption(&menuOption,&event, &running)
        if ( interface==2) 
             LectureEntreeMenuScore(&menuScore,&event, &running) */
      }
      
//----------------Update--------------------------
        if ( interface ==0)
              MiseAJourMenuPrincipal(&menu,&interface);
        /*if ( interface ==1) 
              MiseAJourMenuOption(&menuOption,&interface)
        if ( interface==2) 
             MiseAJourMenuScore(&menuScore,&interface) */     
        SDL_RenderPresent(renderer);
    }
//----------------Free--------------------------
    Liberation(&menu,renderer,window);
