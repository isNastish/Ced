

#include "ceed_platform.h"

internal void game_update_and_render(GameOffscreenBuffer *offscreen_buffer, s32 x_offset, s32 y_offset){
#if OPENGL_RENDERER
    // set background color.
    glClearColor(205.0f/255.0f, 190.0f/255.0f, 112.0f/255.0f, 0);
    // clear the color buffer.
    glClear(GL_COLOR_BUFFER_BIT);
#else
    // NOTE: Draw pattern.
    game_draw_pattern(offscreen_buffer, x_offset, y_offset);
#endif
}

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

internal void game_draw_pattern(GameOffscreenBuffer *buffer, s32 x_offset, s32 y_offset){
    u8 *dest_row = (u8 *)buffer->memory;
    for(s32 y = 0;
        y < buffer->height;
        ++y){
        u32 *dest = (u32 *)dest_row;
        for(s32 x = 0;
            x < buffer->width;
            ++x){
            *dest++ = (112 | (190 << 8) | (205 << 16));
        }
        dest_row += buffer->pitch;
    }
}
