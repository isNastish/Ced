

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

#endif // CED_H
