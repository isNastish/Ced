

#include "ceed_platform.h"

internal void game_draw_square(GameOffscreenBuffer *buffer, V3 *color, u32 left_x, u32 left_y, u32 square_width, u32 square_height){
    u32 *dest = (u32 *)buffer->memory;
    dest += (left_y * buffer->width + left_x);
    for(u32 y = 0; y < square_height; ++y){
        for(u32 x = 0; x < square_width; ++x){
            *dest++ = ((s32)color->b | ((s32)color->g << 8) | ((s32)color->r << 16));
        }
        dest += (buffer->width - square_width);
    }
}

internal void game_fill_background(GameOffscreenBuffer *buffer, V3 *color){
    u8 *dest_row = (u8 *)buffer->memory;
    for(s32 y = 0; y < buffer->height; ++y){
        u32 *dest = (u32 *)dest_row;
        for(s32 x = 0; x < buffer->width; ++x){
            *dest++ = ((s32)color->b | ((s32)color->g << 8) | ((s32)color->r << 16));
        }
        dest_row += buffer->pitch;
    }
}

// TODO:
internal void game_init_stbtt_font(const char *ttf_file_path){
    // TODO: load entire file into memory.
    void *file_contents = 0;
    
    // TODO: call stbtt_InitFont.
    {
        stbtt_fontinfo stb_font_info;
        if(stbtt_InitFont(&stb_font_info, file_contents, stbtt_GetFontOffsetForIndex(file_contents, 0))){
        }
        else{
            // TODO: Error handling ("stbtt_InitFont" call failed).
        }
    }
}

internal void game_update_and_render(GameOffscreenBuffer *offscreen_buffer, s32 x_offset, s32 y_offset){
#if OPENGL_RENDERER
    // set background color.
    glClearColor(205.0f/255.0f, 190.0f/255.0f, 112.0f/255.0f, 0);
    // clear the color buffer.
    glClear(GL_COLOR_BUFFER_BIT);
#else
    u32 center_x = (offscreen_buffer->width / 2);
    u32 center_y = (offscreen_buffer->height / 2);

    V3 back_color = {40.0f, 40.0f, 40.0f};
    V3 square_color = {209.0f, 240.0f, 154.0f};
    game_fill_background(offscreen_buffer, &back_color);
    game_draw_square(offscreen_buffer, &square_color, 660, (center_y + 390), 400, 50);
    
    game_draw_square(offscreen_buffer, &square_color, 400, (center_y + 330), 100, 50);

    u32 x = 510;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 330), 120, 50);
    x += 20;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 100, 50);

    x = 400;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 80, 50);

    x = 490;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);

    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 270), 55, 50);
    

    x = 400;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 70, 50);
    
    x += 80;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);

    x += 65;
    if(key_W){
        V3 tmp = square_color;
        square_color.r = 255.0f;
        square_color.g = 69.0f;
        square_color.b = 0.0f;
        game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
        square_color = tmp;
    }
    else{
        game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    }
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 55, 50);
    x += 65;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 210), 85, 50);

    x = 400;
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;

    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 55, 50);
    x += 65;
    
    game_draw_square(offscreen_buffer, &square_color, x, (center_y + 150), 100, 50);
#endif
}
