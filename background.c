#include <stdio.h>
#include "header.h"

// Global background instance
Background background = {NULL, {0, 0, 0, 0}, 0, 0, NULL, 32};

// Initialize the background by loading an image
int init_background(SDL_Renderer* renderer, const char* image_path) {
    printf("Loading background image: %s\n", image_path);
    
    // Load the image as a surface
    SDL_Surface* surface = IMG_Load(image_path);
    if (!surface) {
        printf("Failed to load background image: %s\n", IMG_GetError());
        return -1;
    }
    
    // Store original dimensions
    background.width = surface->w;
    background.height = surface->h;
    printf("Background image size: %dx%d\n", background.width, background.height);
    
    // Create texture from surface
    background.texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!background.texture) {
        printf("Failed to create texture: %s\n", SDL_GetError());
        return -1;
    }
    
    // Set destination rectangle (where to draw on screen)
    // By default, draw at position (0, 0) with original size
    background.dest_rect.x = 0;
    background.dest_rect.y = 0;
    background.dest_rect.w = background.width;
    background.dest_rect.h = background.height;
    
    printf("Background initialized successfully\n");
    return 0;
}

// Draw the background
void draw_background(SDL_Renderer* renderer) {
    if (background.texture) {
        SDL_RenderCopy(renderer, background.texture, NULL, &background.dest_rect);
    }
}

// Update background state (if animated/scrolling)
void update_background(void) {
    // Add your background update logic here
    // e.g., scrolling backgrounds, parallax effects, etc.
}

// Clean up background resources
void cleanup_background(void) {
    if (background.texture) {
        SDL_DestroyTexture(background.texture);
        background.texture = NULL;
        printf("Background texture destroyed\n");
    }
    
    // Stop and free music
    if (background.music) {
        Mix_HaltMusic();
        Mix_FreeMusic(background.music);
        background.music = NULL;
        printf("Background music freed\n");
    }
}

// Initialize background music
int init_background_music(const char* music_path, int volume) {
    printf("Loading background music: %s\n", music_path);
    
    // Load music file
    background.music = Mix_LoadMUS(music_path);
    if (!background.music) {
        printf("Failed to load background music: %s\n", Mix_GetError());
        return -1;
    }
    
    // Set volume (0-128, where 128 is max)
    // For "low volume you can hear without feeling it", use around 25-35% of max
    background.music_volume = volume;
    Mix_VolumeMusic(background.music_volume);
    
    printf("Background music loaded successfully (volume: %d/128)\n", volume);
    return 0;
}

// Play background music on loop
void play_background_music(void) {
    if (background.music) {
        // -1 means loop forever
        if (Mix_PlayMusic(background.music, -1) == -1) {
            printf("Failed to play music: %s\n", Mix_GetError());
        } else {
            printf("Background music started (looping)\n");
        }
    }
}

// Stop background music
void stop_background_music(void) {
    Mix_HaltMusic();
    printf("Background music stopped\n");
}

// Pause background music
void pause_background_music(void) {
    Mix_PauseMusic();
    printf("Background music paused\n");
}

// Resume background music
void resume_background_music(void) {
    Mix_ResumeMusic();
    printf("Background music resumed\n");
}

// Set background music volume (0-128)
void set_background_music_volume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 128) volume = 128;
    
    background.music_volume = volume;
    Mix_VolumeMusic(background.music_volume);
    printf("Background music volume set to: %d/128 (%d%%)\n", volume, (volume * 100) / 128);
}

// Get current music volume
int get_background_music_volume(void) {
    return background.music_volume;
}

// Check if music is currently playing
int is_music_playing(void) {
    return Mix_PlayingMusic();
}

// Check if music is paused
int is_music_paused(void) {
    return Mix_PausedMusic();
}
