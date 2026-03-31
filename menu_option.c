/* Legacy copy of the options menu.
 * See the `menuSDL/` directory for the cleaned-up implementation
 * where initialization and asset management have been modularized.
 */

#include "menu.h"

int mouse_over(SDL_Rect r,int x,int y)
{
    return (x>=r.x && x<=r.x+r.w &&
            y>=r.y && y<=r.y+r.h);
}

int menu(SDL_Window *window, SDL_Renderer *renderer)
{
    SDL_Event e;
    int quit = 0;
    int fullscreen = 0;

    /* ---------- LOAD IMAGES ---------- */

    SDL_Texture *background =
        IMG_LoadTexture(renderer,"options.png");

    SDL_Texture *plus =
        IMG_LoadTexture(renderer,"volumeplus.png");

    SDL_Texture *minus =
        IMG_LoadTexture(renderer,"volumeminus.png");

    SDL_Texture *mute =
        IMG_LoadTexture(renderer,"volumemute.png");

    SDL_Texture *full =
        IMG_LoadTexture(renderer,"fullscreen.png");

    SDL_Texture *normal =
        IMG_LoadTexture(renderer,"normalscreen.png");

    /* ---------- MUSIC ---------- */

    Mix_Music *music = Mix_LoadMUS("music.mp3");
    Mix_Chunk *click = Mix_LoadWAV("sonbref.wav");

    Mix_PlayMusic(music,-1);

    /* ---------- FONT ---------- */

    TTF_Font *font =
        TTF_OpenFont("arial.ttf",60);

    SDL_Color white={255,255,255};

    SDL_Surface *surf =
        TTF_RenderText_Blended(font,"OPTIONS",white);

    SDL_Texture *title =
        SDL_CreateTextureFromSurface(renderer,surf);

    SDL_FreeSurface(surf);

    SDL_Rect titleRect={0,40,400,80};

    /* ---------- BUTTON SIZE ---------- */

    int bw=80;
    int bh=80;

    SDL_Rect btnPlus,btnMinus,btnMute,btnFull;

    while(!quit)
    {
        int w,h;
        SDL_GetWindowSize(window,&w,&h);

        /* ✅ CENTER BUTTONS EVERY FRAME */

        int centerX=w/2;

        btnMinus=(SDL_Rect){centerX-180,h/2,bw,bh};
        btnMute =(SDL_Rect){centerX-40 ,h/2,bw,bh};
        btnPlus =(SDL_Rect){centerX+100,h/2,bw,bh};
        btnFull =(SDL_Rect){centerX-40 ,h/2+120,bw,bh};

        titleRect.x=centerX-200;

        /* ---------- EVENTS ---------- */

        while(SDL_PollEvent(&e))
        {
            if(e.type==SDL_QUIT)
                quit=1;

            if(e.type==SDL_KEYDOWN &&
               e.key.keysym.sym==SDLK_ESCAPE)
                quit=1;

            if(e.type==SDL_MOUSEBUTTONDOWN)
            {
                int mx=e.button.x;
                int my=e.button.y;

                Mix_PlayChannel(-1,click,0);

                if(mouse_over(btnPlus,mx,my))
                    Mix_VolumeMusic(Mix_VolumeMusic(-1)+10);

                if(mouse_over(btnMinus,mx,my))
                    Mix_VolumeMusic(Mix_VolumeMusic(-1)-10);

                if(mouse_over(btnMute,mx,my))
                    Mix_VolumeMusic(0);

                if(mouse_over(btnFull,mx,my))
                {
                    fullscreen=!fullscreen;

                    if(fullscreen)
                        SDL_SetWindowFullscreen(
                            window,
                            SDL_WINDOW_FULLSCREEN_DESKTOP);
                    else
                        SDL_SetWindowFullscreen(window,0);
                }
            }
        }

        /* ---------- DRAW ---------- */

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer,background,NULL,NULL);

        SDL_RenderCopy(renderer,title,NULL,&titleRect);

        SDL_RenderCopy(renderer,minus,NULL,&btnMinus);
        SDL_RenderCopy(renderer,mute,NULL,&btnMute);
        SDL_RenderCopy(renderer,plus,NULL,&btnPlus);

        if(fullscreen)
            SDL_RenderCopy(renderer,normal,NULL,&btnFull);
        else
            SDL_RenderCopy(renderer,full,NULL,&btnFull);

        SDL_RenderPresent(renderer);
    }

    /* ---------- FREE ---------- */

    SDL_DestroyTexture(background);
    SDL_DestroyTexture(plus);
    SDL_DestroyTexture(minus);
    SDL_DestroyTexture(mute);
    SDL_DestroyTexture(full);
    SDL_DestroyTexture(normal);
    SDL_DestroyTexture(title);

    Mix_FreeMusic(music);
    Mix_FreeChunk(click);

    TTF_CloseFont(font);

    return 0;
}
