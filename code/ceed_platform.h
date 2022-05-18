

#if !defined(CEED_PLATFORM_H)
#define CEED_PLATFORM_H

typedef union V2 V2;
union V2
{
    struct
    {
        f32 x1;
        f32 x2;
    };

    struct
    {
        f32 x1;
        f32 x2;
    }u;
    
    f32 x[2];
};

internal void game_update_and_render();

#endif // CEED_PLATFORM_H
