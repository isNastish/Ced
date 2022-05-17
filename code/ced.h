

#if !defined(CED_H)
#define CED_H

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

#endif // CED_H
