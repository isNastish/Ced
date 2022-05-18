

#if !defined(CEED_PLATFORM_H)
#define CEED_PLATFORM_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "external/stb_truetype.h"

#define global static
#define internal static
#define local_persist static

#define const
#define Pi32 3.14159265358f
#define Square(x) (x)*(x)
#define Cube(x) (x)*(x)*(x)

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 bool32;
typedef bool32 b32;

typedef float real32;
typedef double real64;

typedef real32 f32;
typedef real64 f64;

typedef s32 b32;

global u8 key_W;
global u8 key_A;
global u8 key_S;
global u8 key_D;

typedef union V2 V2;
union V2
{
    struct
    {
        f32 x1;
        f32 x2;
    };
    f32 x[2];
};

typedef union V3 V3;
union V3
{
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };
    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };
    f32 xyz[3];
    f32 rgb[3];
};

typedef struct GameOffscreenBuffer GameOffscreenBuffer;
struct GameOffscreenBuffer{
    void *memory;
    u32 width;
    u32 height;
    u32 pitch;
};

typedef struct GameSoundBuffer GameSoundBuffer;
struct GameSoundBuffer{
    s32 samples_per_second;
    s32 sample_count;
    s16 *samples;
};

internal void game_update_and_render(GameOffscreenBuffer *buffer, s32 x_offset, s32 y_offset);
internal void game_init_stbtt_font(const char *ttf_file_path);
internal void game_draw_pattern(GameOffscreenBuffer *offscreen_buffer, s32 x_offset, s32 y_offset);

#endif // CEED_PLATFORM_H
