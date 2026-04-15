#include <stddef.h>
#include <string.h>

typedef enum {
    MOVEMENT_WALK = 0,
    MOVEMENT_RUN,
    MOVEMENT_DANCE,
    MOVEMENT_JUMP,
    MOVEMENT_COUNT
} MovementType;

typedef struct {
    const char *name;
    const char *sprite_sheet;
    int rows;
    int cols;
    int frame_duration_ms;
    float speed_x;
    float speed_y;
    int loops;
} AnimationMovement;

static const AnimationMovement kAnimationMovements[MOVEMENT_COUNT] = {
    [MOVEMENT_WALK] = {
        .name = "walk",
        .sprite_sheet = "spritesheet_characters/mr_harry_walk_cycle_transparent.png",
        .rows = 6,
        .cols = 5,
        .frame_duration_ms = 110,
        .speed_x = 2.5f,
        .speed_y = 0.0f,
        .loops = 1
    },
    [MOVEMENT_RUN] = {
        .name = "run",
        .sprite_sheet = "spritesheet_characters/mr_harry_walk_cycle_transparent.png",
        .rows = 6,
        .cols = 5,
        .frame_duration_ms = 60,
        .speed_x = 5.0f,
        .speed_y = 0.0f,
        .loops = 1
    },
    [MOVEMENT_DANCE] = {
        .name = "dance",
        .sprite_sheet = "spritesheet_characters/mr_harry_dance_animation_transparent.png",
        .rows = 6,
        .cols = 5,
        .frame_duration_ms = 80,
        .speed_x = 0.0f,
        .speed_y = 0.0f,
        .loops = 1
    },
    [MOVEMENT_JUMP] = {
        .name = "jump",
        .sprite_sheet = "spritesheet_characters/mr_harry_stops.png",
        .rows = 1,
        .cols = 1,
        .frame_duration_ms = 140,
        .speed_x = 1.0f,
        .speed_y = -8.0f,
        .loops = 0
    }
};

const AnimationMovement *animation_get_movement(MovementType type)
{
    if (type < 0 || type >= MOVEMENT_COUNT) return NULL;
    return &kAnimationMovements[type];
}

const AnimationMovement *animation_find_movement(const char *name)
{
    size_t i;

    if (!name) return NULL;

    for (i = 0; i < MOVEMENT_COUNT; ++i) {
        if (strcmp(kAnimationMovements[i].name, name) == 0) {
            return &kAnimationMovements[i];
        }
    }

    return NULL;
}

size_t animation_movement_count(void)
{
    return MOVEMENT_COUNT;
}
