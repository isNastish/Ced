


/*
  TODO:

  [x] Timing (QueryPerformanceCounter, RDTSC).
  [x] Support bitmap rendering to test controller input.
  [x] Support rendering of bitmaps as well as OpenGL rendering.
  [x] Fullscreen mode.
  [x] Initialize DirectSound and output Square/Sine wave to the sound buffer.
  [x] Mouse position.
  [] Mouse input.
  [] Gamepad input (deal with DEADZONEs), and test the input.
  [] Raw input (support for multiple keyboars).
  [] WM_ACTIVATEAPP (handle case when application is not active).
  [] Proper OpenGL initialization.
  [] Stuby about true type font.
  [] Figure out what's going on in stb_truetype.h
  [] Implement corrent text renderer (with kerning table).
  ...
  
  
 */

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <gl/gl.h>
 
#define OPENGL_RENDERER 1
#define XINPUT_THUMB_STICK_DEADZONE 32767.0f
#define Out(s) OutputDebugStringA(s);
#define Out_s(buf, buf_size, format, ...)           \
    sprintf_s(buf, buf_size, format, __VA_ARGS__);  \
    Out(buf);


// NOTE: my include files:
#include "ceed_platform.c"

typedef struct W32_WindowDimension W32_WindowDimension;
struct W32_WindowDimension{
    s32 width;
    s32 height;
};

typedef struct W32_OffscreenBuffer W32_OffscreenBuffer;
struct W32_OffscreenBuffer{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    u32 pitch;
};

typedef struct W32_SoundOutput W32_SoundOutput;
struct W32_SoundOutput{
    s32 samples_per_second;
    s32 bytes_per_sample;
    s32 buffer_size;
    s32 tone_volume;
    s32 tone_hz;
    s32 wave_period;
    u32 running_sample_index;
};

global b32 global_running;
global HDC global_device_context;
global W32_OffscreenBuffer global_offscreen_buffer;
global LPDIRECTSOUNDBUFFER global_secondary_sound_buffer;

internal W32_WindowDimension w32_get_window_dimension(HWND window_handle);
internal void w32_resize_dib_section(W32_OffscreenBuffer *offscree_buffer, s32 window_width, s32 window_height);
internal void w32_display_offscreen_buffer(W32_OffscreenBuffer *offscreen_buffer, HDC device_context, s32 window_width, s32 window_height);
internal void w32_toggle_fullscreen(HWND window_handle);
internal V2 w32_get_mouse_position(HWND window_handle);
internal void w32_init_xinput(void);
internal void w32_init_opengl(HWND window_handle);
internal void w32_init_dsound(HWND window_handle, s32 samples_per_second, s32 bytes_per_sample);
internal void w32_fill_sound_buffer(W32_SoundOutput *sound_output, s32 byte_to_lock, s32 bytes_to_write);
inline internal u64 w32_timer_get_frequency(void);
inline internal u64 w32_timer_get_counts(void);
inline internal u64 w32_timer_cycles(void);


inline internal u64 w32_timer_get_cycles(void){
    u64 result = __rdtsc();
    return(result);
}

inline internal u64 w32_timer_get_frequency(void){
    u64 result = 0;
    QueryPerformanceFrequency((LARGE_INTEGER *)&result);
    return(result);
}

inline internal u64 w32_timer_get_counts(void){
    u64 result = 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&result);
    return(result);
}


internal void w32_fill_sound_buffer(W32_SoundOutput *sound_output, s32 byte_to_lock, s32 bytes_to_write){
    local_persist s32 dsound_is_playing = 0;
    
    void *locked_region1 = 0;
    void *locked_region2 = 0;
    u32 locked_region1_size = 0;
    u32 locked_region2_size = 0;

    if(IDirectSoundBuffer_Lock(global_secondary_sound_buffer, byte_to_lock, bytes_to_write,
                               &locked_region1, &locked_region1_size,
                               &locked_region2, &locked_region2_size, 0) == 0){

        // Structure of the sound buffer.
        // s16 s16  s16 s16  s16 s16  s16 s16  s16 s16  s16 s16  s16 s16  s16 s16 ...
        // [L   R]  [L   R]  [L   R]  [L   R]  [L   R]  [L   R]  [L   R]  [L   R] ... 
        // Each sample is 32bits (16 bit for Left channel and 16 bits for right channel).
        s16 *sample_out = (s16 *)locked_region1;
        u32 samples_count_region1 = (locked_region1_size / sound_output->bytes_per_sample);
        u32 samples_count_region2 = (locked_region2_size / sound_output->bytes_per_sample);
        for(u32 sample_index = 0;
            sample_index < samples_count_region1;
            ++sample_index){
            f32 t = 2.0f * Pi32 * ((f32)sound_output->running_sample_index / (f32)sound_output->wave_period);
            f32 sin_value = sinf(t);
            s16 sample_value = (s16)(sin_value * sound_output->tone_volume);
            *sample_out++ = sample_value; // LEFT channel.
            *sample_out++ = sample_value; // RIGHT channel.
            ++sound_output->running_sample_index;
        }

        if(locked_region2){
            sample_out = (s16 *)locked_region2;
            for(u32 sample_index = 0;
                sample_index < samples_count_region2;
                ++sample_index){
                f32 t = 2.0f * Pi32 * ((f32)sound_output->running_sample_index / (f32)sound_output->wave_period);
                f32 sin_value = sinf(t);
                s16 sample_value = (s16)(sin_value * sound_output->tone_volume);
                *sample_out++ = sample_value; // LEFT channel.
                *sample_out++ = sample_value; // RIGHT channel.
                ++sound_output->running_sample_index;
            }
        }
        IDirectSoundBuffer_Unlock(global_secondary_sound_buffer, locked_region1, locked_region1_size,
                                  locked_region2, locked_region2_size);
        if(!dsound_is_playing){
            IDirectSoundBuffer_Play(global_secondary_sound_buffer, 0, 0, DSBPLAY_LOOPING);
            dsound_is_playing = 1;
        }
    }
    else{
        // TODO: Error handling ("IDirectSoundBuffer_Lock" call failed).
    }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pc_guid_device, LPDIRECTSOUND *pp_ds, LPUNKNOWN p_unk_outer)
typedef DIRECT_SOUND_CREATE(direct_sound_create);
internal void w32_init_dsound(HWND window_handle, s32 samples_per_second, s32 bytes_per_sample){
    // NOTE: LoadLibraryA();
    HMODULE dsound_lib = LoadLibraryA("dsound.dll");
    if(dsound_lib){
        direct_sound_create *DirectSoundCreate =
            (direct_sound_create *)GetProcAddress(dsound_lib, "DirectSoundCreate");
        LPDIRECTSOUND direct_sound = {0};
        if(DirectSoundCreate && (DirectSoundCreate(0, &direct_sound, 0) == 0)){
            WAVEFORMATEX wave_format = {0};
            {
                wave_format.wFormatTag = WAVE_FORMAT_PCM;
                wave_format.nChannels = 2;
                wave_format.nSamplesPerSec = samples_per_second;
                wave_format.wBitsPerSample = 16;
                wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
                wave_format.nAvgBytesPerSec = (wave_format.nSamplesPerSec * wave_format.nBlockAlign);
                wave_format.cbSize = 0;
            }
            if(IDirectSound_SetCooperativeLevel(direct_sound, window_handle, DSSCL_PRIORITY) == 0){
                DSBUFFERDESC buffer_desc = {0};
                {
                    buffer_desc.dwSize = sizeof(DSBUFFERDESC);
                    buffer_desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
                    buffer_desc.dwBufferBytes = 0;
                    buffer_desc.dwReserved = 0;
                    buffer_desc.lpwfxFormat = 0;
                    buffer_desc.guid3DAlgorithm = GUID_NULL;
                }
                LPDIRECTSOUNDBUFFER direct_sound_buffer = {0};
                if(IDirectSound_CreateSoundBuffer(direct_sound, &buffer_desc, &direct_sound_buffer, 0) == 0){
                    if(IDirectSoundBuffer_SetFormat(direct_sound_buffer, &wave_format) == 0){
                        Out("success!\n");
                    }
                    else{
                        // TODO: Error handling("IDirectSoundBuffer_SetFormat" call failed).
                    }
                }
                else{
                    // TODO: Error handling("IDirectSound_CreateSoundBuffer" call failed).
                }
            }
            else{
                // TODO: Error handling("IDirectSound_SetCooperativeLevel" call failed).
            }
            DSBUFFERDESC buffer_desc = {0};
            {
                buffer_desc.dwSize = sizeof(DSBUFFERDESC);
                buffer_desc.dwFlags = 0;
                buffer_desc.dwBufferBytes = (samples_per_second * bytes_per_sample);
                buffer_desc.dwReserved = 0;
                buffer_desc.lpwfxFormat = &wave_format;
                buffer_desc.guid3DAlgorithm = GUID_NULL;
            }
            if(IDirectSound_CreateSoundBuffer(direct_sound, &buffer_desc, &global_secondary_sound_buffer, 0) == 0){
                Out("success!\n");
            }
            else{
                // TODO: Error handling("IDirectSound_CreateSoundBuffer" call failed).
            }
        }
        else{
            // TODO: Error handling ("DirectSoundCreate" call failed).
        }
    }
    else{
        // TODO: Error handling (failed to load "dsound.dll").
    }
}

internal V2 w32_get_mouse_pos(HWND window_handle){
    V2 mouse_pos = {0};
    POINT cursor_pos = {0};
    GetCursorPos(&cursor_pos);
    ScreenToClient(window_handle, &cursor_pos);
    mouse_pos.x1 = cursor_pos.x;
    mouse_pos.x2 = cursor_pos.y;
    return(mouse_pos);
}

internal void w32_toggle_fullscreen(HWND window_handle){
    local_persist WINDOWPLACEMENT window_placement = {0};
    window_placement.length = sizeof(WINDOWPLACEMENT);
    u32 window_style = GetWindowLongA(window_handle, GWL_STYLE);

    if(window_style & WS_OVERLAPPEDWINDOW){
        // NOTE: Go to fullscreen mode.
        MONITORINFO monitor_info = {0};
        monitor_info.cbSize = sizeof(MONITORINFO);
        
        if(GetWindowPlacement(window_handle, &window_placement) &&
           GetMonitorInfo(MonitorFromWindow(window_handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info)){
            SetWindowLongA(window_handle, GWL_STYLE, (window_style & ~WS_OVERLAPPEDWINDOW));
            SetWindowPos(window_handle, HWND_TOP, monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
        }
        else{
            // TODO: Error handling ("GetWindowPlacement" or "GetMonitorInfo" calls failed).
        }
    }
    else{
        // NOTE: Go to normal mode.
        SetWindowLongA(window_handle, GWL_STYLE, (window_style | WS_OVERLAPPEDWINDOW)); // NOTE: (WS_VISIBLE | WS_CLIPSIBLINGS).
        SetWindowPlacement(window_handle, &window_placement);
        SetWindowPos(window_handle, 0, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER));
    }
}

// TODO: Init OpenGL more properly.
internal void w32_init_opengl(HWND window_handle){
    s32 pixel_format = 0;
    PIXELFORMATDESCRIPTOR pfd = {0};
    {
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA; // NOTE: the kind of framebuffer, RGBA or palette.
        pfd.cColorBits = 32; // NOTE: Color depth of the framebuffer.
        pfd.cDepthBits = 24; // NOTE: Number of bits for the depthbuffer.
        pfd.cStencilBits = 8; // NOTE: Number of bits for the stencilbuffer.
        pfd.iLayerType = PFD_MAIN_PLANE;
    }

    if((pixel_format = ChoosePixelFormat(global_device_context, &pfd)) != 0){
        if(SetPixelFormat(global_device_context, pixel_format, &pfd) == TRUE){
            HGLRC opengl_dummy_context = wglCreateContext(global_device_context);
        
            if(wglMakeCurrent(global_device_context, opengl_dummy_context)){
                char buf[128];
                Out_s(buf, sizeof(buf), "OpenGL version: %s.\n", (char *)glGetString(GL_VERSION));
            }
            else{
                // TODO: error handling.
            }
        }
        else{
            // TODO: error handling.
        }
    }
    else{
        // TODO: error handling.
    }
}

#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dw_user_index, XINPUT_STATE* p_state)
typedef XINPUT_GET_STATE(xinput_get_state);
XINPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global xinput_get_state *XInputGetStatePtr = XInputGetStateStub;
#define XInputGetState XInputGetStatePtr

#define XINPUT_SET_STATE(name) DWORD WINAPI name(DWORD dw_user_index, XINPUT_VIBRATION* p_vibration)
typedef XINPUT_SET_STATE(xinput_set_state);
XINPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global xinput_set_state *XInputSetStatePtr = XInputSetStateStub;
#define XInputSetState XInputSetStatePtr

internal void w32_init_xinput(void){
    HMODULE xinput_lib = LoadLibraryA("xinput1_4.dll");
    if(!xinput_lib){
        xinput_lib = LoadLibraryA("xinput9_1_0.dll");
    }
    if(!xinput_lib){
        xinput_lib = LoadLibraryA("xinput1_3.dll");
    }

    if(xinput_lib){
        XInputGetStatePtr = (xinput_get_state *)GetProcAddress(xinput_lib, "XInputGetState");
        if(!XInputGetStatePtr){
            XInputGetStatePtr = XInputGetStateStub;
        }
        XInputSetStatePtr = (xinput_set_state *)GetProcAddress(xinput_lib, "XInputSetState");
        if(!XInputSetStatePtr){
            XInputSetStatePtr = XInputSetStateStub;
        }
    }
    else{
        // TODO: error handling (unable to load xinput.dll).
    }
}

internal W32_WindowDimension w32_get_window_dimension(HWND window_handle){
    W32_WindowDimension window_dimension = {0};
    
    RECT client_rect = {0};
    GetClientRect(window_handle, &client_rect);
    window_dimension.width = client_rect.right - client_rect.left;
    window_dimension.height = client_rect.bottom - client_rect.top;
    
    return(window_dimension);
}

internal void w32_resize_dib_section(W32_OffscreenBuffer *offscreen_buffer, s32 window_width, s32 window_height){
    if(offscreen_buffer->memory){
        VirtualFree(offscreen_buffer->memory, 0, MEM_RELEASE);
    }

    offscreen_buffer->width = window_width;
    offscreen_buffer->height = window_height;
    
    offscreen_buffer->info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    offscreen_buffer->info.bmiHeader.biWidth = offscreen_buffer->width;
    offscreen_buffer->info.bmiHeader.biHeight = -offscreen_buffer->height; // top-down bitmap, origin in the upper-left corner.
    offscreen_buffer->info.bmiHeader.biPlanes = 1;
    offscreen_buffer->info.bmiHeader.biBitCount = 32;
    offscreen_buffer->info.bmiHeader.biCompression = BI_RGB;
    offscreen_buffer->info.bmiHeader.biSizeImage = 0;
    offscreen_buffer->info.bmiHeader.biXPelsPerMeter = 0;
    offscreen_buffer->info.bmiHeader.biYPelsPerMeter = 0;
    offscreen_buffer->info.bmiHeader.biClrUsed = 0;
    offscreen_buffer->info.bmiHeader.biClrImportant = 0;

    {
        u32 bpp = 4; // bytes per pixel.
        u32 allocation_size = (offscreen_buffer->width * offscreen_buffer->height) * bpp;
        offscreen_buffer->memory = VirtualAlloc(0, allocation_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        offscreen_buffer->pitch = (offscreen_buffer->width * bpp);
    }
}

internal void w32_display_offscreen_buffer(W32_OffscreenBuffer *offscreen_buffer, HDC device_context,
                                           s32 window_width, s32 window_height){
    s32 scanlines_count = StretchDIBits(device_context, 0, 0, window_width, window_height,
                                        0, 0, offscreen_buffer->width, offscreen_buffer->height,
                                        offscreen_buffer->memory, &offscreen_buffer->info,
                                        DIB_RGB_COLORS, SRCCOPY);
    assert(scanlines_count == offscreen_buffer->height);
}

LRESULT CALLBACK w32_main_window_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param){
    LRESULT result = 0;
    local_persist fullscreen_mode = 0;
    
    switch(message){
        case(WM_ACTIVATEAPP):{
            global_device_context = GetDC(window_handle);
        }break;
        case(WM_CLOSE):{
            global_running = 0;
        }break;
        case(WM_DESTROY ):{
            PostQuitMessage(0);
        }break;
        case(WM_SYSKEYDOWN):
        case(WM_SYSKEYUP):
        case(WM_KEYDOWN):
        case(WM_KEYUP):{
            s32 vk_code = w_param;
            s32 was_down = ((l_param & (1 << 30)) != 0);
            s32 is_down = ((l_param & (1 << 31)) == 0);

            //
            // TODO: Process some keys! Later, we'll do more.
            //
            if(vk_code == 'W'){
                key_W = 1;
            }
            else if(vk_code == 'A'){
                key_A = 1;
            }
            else if(vk_code == 'S'){
                key_S = 1;
            }
            else if(vk_code == 'D'){
                key_D = 1;
            }
            
            result = DefWindowProc(window_handle, message, w_param, l_param);
        }break;
        case(WM_LBUTTONUP):{
            w32_toggle_fullscreen(window_handle);
        }break;
        case(WM_SIZE):{
#if OPENGL_RENDERER
            // NOTE: Render using OpenGL.
            V3 color = {188.0f, 238.0f, 104.0f};
            glClearColor(color.r/255.0f, color.g/255.0f, color.b/255.0f, 0);
            // clear the color buffer.
            glClear(GL_COLOR_BUFFER_BIT);
            SwapBuffers(global_device_context);
#else
            // NOTE: Render using bitmaps.
            W32_WindowDimension window_dimension = w32_get_window_dimension(window_handle);
            w32_resize_dib_section(&global_offscreen_buffer, window_dimension.width, window_dimension.height);
#endif
        };
        default:{
            result = DefWindowProc(window_handle, message, w_param, l_param);
        }break;
    }
    return(result);
}


INT WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, PSTR cmd_line, int show_code){
    u64 timer_frequency = w32_timer_get_frequency();
    w32_init_xinput();
    
    WNDCLASSA window_class = {0};
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        window_class.lpfnWndProc = w32_main_window_proc;
        window_class.hInstance = instance;
        //window_class.hIcon = ;
        //window_class.hCursor = ;
        window_class.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
        window_class.lpszClassName = "AppWindowClass";
    }
    
    if(RegisterClassA(&window_class)){
        HWND window_handle;
        if((window_handle = CreateWindowEx(0, window_class.lpszClassName, "Ced", (WS_OVERLAPPEDWINDOW | WS_VISIBLE),
                                           CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0)) != 0){
            w32_init_opengl(window_handle);
            
            // NOTE: DirectSound:
            W32_SoundOutput sound_output = {0};
            {
                sound_output.samples_per_second = 48000;
                sound_output.bytes_per_sample = sizeof(s16) * 2;
                sound_output.buffer_size = (sound_output.samples_per_second * sound_output.bytes_per_sample);
                sound_output.tone_volume = 400;
                sound_output.tone_hz = 256; // middle c.
                sound_output.running_sample_index = 0;
                sound_output.wave_period = (sound_output.samples_per_second / sound_output.tone_hz);
            }
            w32_init_dsound(window_handle, sound_output.samples_per_second, sound_output.bytes_per_sample);
            w32_fill_sound_buffer(&sound_output, 0, sound_output.buffer_size);

            s32 x_offset = 0;
            s32 y_offset = 0;

            global_running = 1;
            u64 timer_start_counts = w32_timer_get_counts(),
                timer_end_counts;
            u64 timer_start_cycles = w32_timer_get_cycles(),
                timer_end_cycles;
            
            while(global_running){
                MSG message;
        
                while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }
        
                {
                    // NOTE: XInput:
                    s32 gamepad_id;
                    for(gamepad_id = 0;
                        gamepad_id < XUSER_MAX_COUNT;
                        ++gamepad_id){
                        XINPUT_STATE controller_state = {0};

                        if(XInputGetState(gamepad_id, &controller_state) == ERROR_SUCCESS){
                            // NOTE: Controller is connected.
                            XINPUT_GAMEPAD *pad = &controller_state.Gamepad;

                            b32 dpad_up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                            b32 dpad_down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                            b32 dpad_left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                            b32 dpad_right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                
                            b32 start = (pad->wButtons & XINPUT_GAMEPAD_START);
                            b32 back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
                
                            b32 left_thumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
                            b32 right_thumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
                
                            b32 left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                            b32 right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

                            b32 A_button = (pad->wButtons & XINPUT_GAMEPAD_A);
                            b32 B_button = (pad->wButtons & XINPUT_GAMEPAD_B);
                            b32 X_button = (pad->wButtons & XINPUT_GAMEPAD_X);
                            b32 Y_button = (pad->wButtons & XINPUT_GAMEPAD_Y);

                            b32 left_trigger = pad->bLeftTrigger;
                            b32 right_trigger = pad->bRightTrigger;

                            s16 left_thumbstick_x = pad->sThumbLX;
                            s16 left_thumbstick_y = pad->sThumbLY;
                            s16 right_thumbstick_x = pad->sThumbRX;
                            s16 right_thumbstick_y = pad->sThumbRY;

                            // (A|B|X|Y) buttons test:
                            {
                                if(A_button){ y_offset -= 2; }
                                else if(B_button) { x_offset -= 2; }
                                else if(X_button) { x_offset += 2; }
                                else if(Y_button) { y_offset += 2; }
                            }

                            // (L|R) trigger:
                            {
                                if(left_trigger){ x_offset += 2; }
                                else if(right_trigger) { x_offset -= 2; }
                                
                                char buf[32];
                                Out_s(buf, sizeof(buf), "L trigger: %i\nR trigger: %i\n\n", left_trigger, right_trigger);
                            }
                            
                            // (L|R) thumbstick test:
                            {
                                //
                                f32 lx = left_thumbstick_x;
                                f32 ly = left_thumbstick_y;
                                
                                f32 magnitude = sqrt(Square(lx) + Square(ly));

                                
                                // in range [0.0, ..., 1.0f].
                                f32 normalized_magnitude = 0.0f;
                                if(magnitude > XINPUT_THUMB_STICK_DEADZONE){
                                    magnitude = XINPUT_THUMB_STICK_DEADZONE;
                                }
                                f32 normalized_lx = 0.0;
                                f32 normalized_ly = 0.0;
                                
                                if(lx > abs(XINPUT_THUMB_STICK_DEADZONE)){
                                    lx = (lx > 0) ? 32767.0f : -32767.0f;
                                }
                                if(ly > abs(XINPUT_THUMB_STICK_DEADZONE)){
                                    ly = (ly > 0) ? 32767.0f : -32767.0f;
                                }
                                
                                if(magnitude != 0){
                                    
                                    normalized_lx = lx / magnitude;
                                    normalized_ly = ly / magnitude;
                                }
                                normalized_magnitude = magnitude / XINPUT_THUMB_STICK_DEADZONE;


                                // direction:
                                if((normalized_lx > 0) && (normalized_ly > 0)){ // 1 quadrant (+, +).
                                    x_offset -= 2;
                                    y_offset += 2;
                                }
                                else if((normalized_lx < 0) && (normalized_ly > 0)){ // 2 quadrant (-, +).
                                    x_offset += 2;
                                    y_offset += 2;
                                }
                                else if((normalized_lx < 0) && (normalized_ly < 0)){ // 3 quadrant (-, -).
                                    x_offset += 2;
                                    y_offset -= 2;
                                }
                                else if((normalized_ly > 0) && (normalized_ly < 0)){ // 4 quadrant (+, -).
                                    x_offset -= 2;
                                    y_offset -= 2;
                                }
                                
                                char buf[256];
                                Out_s(buf, sizeof(buf),
                                      "lx: %f\nly: %f\nnor_lx: %f\nnorm_ly: %f\nmagnitude: %f\nnorm_magnitude: %f\n\n",
                                      lx, ly, normalized_lx, normalized_ly, magnitude, normalized_magnitude);
                            }
                        }
                        else{
                            // NOTE: Controller is not connected.
                        }

                        // NOTE: Controller test.
#if 0
                        XINPUT_VIBRATION xinput_vibration = {0};
                        {
                            xinput_vibration.wLeftMotorSpeed = 65535;
                            xinput_vibration.wRightMotorSpeed = 65535;
                        }
                        XInputSetState(gamepad_id, &xinput_vibration);
#endif
                    }
                }

                {
                    // NOTE: DirectSound:
                    u32 play_cursor = 0;
                    u32 write_cursor = 0;
                    if(IDirectSoundBuffer_GetCurrentPosition(global_secondary_sound_buffer, &play_cursor, &write_cursor) == 0){
                        u32 byte_to_lock =
                            (sound_output.running_sample_index * sound_output.bytes_per_sample) % sound_output.buffer_size;
                        u32 bytes_to_write = 0;

                        if(byte_to_lock > play_cursor){
                            bytes_to_write = (sound_output.buffer_size - byte_to_lock);
                            bytes_to_write += play_cursor;
                        }
                        else{
                            bytes_to_write = (play_cursor - byte_to_lock);
                        }

                        w32_fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write);
                    }
                    else{
                        // TODO: Error handling ("IDirectSoundBuffer_GetCurrentPositino" call failed).
                    }
                }
                                
                {
                    // NOTE: Game update and render:
                    GameOffscreenBuffer offscreen_buffer = {0};
                    {
                        offscreen_buffer.memory = global_offscreen_buffer.memory;
                        offscreen_buffer.width = global_offscreen_buffer.width;
                        offscreen_buffer.height = global_offscreen_buffer.height;
                        offscreen_buffer.pitch = global_offscreen_buffer.pitch;
                    }
                    game_update_and_render(&offscreen_buffer, x_offset, y_offset);
#if OPENGL_RENDERER
                    SwapBuffers(global_device_context);
#else
                    W32_WindowDimension window_dimension = w32_get_window_dimension(window_handle);
                    w32_display_offscreen_buffer(&global_offscreen_buffer, global_device_context,
                                                 window_dimension.width, window_dimension.height);
#endif
                }

                {
                    // NOTE: Timing.
                    timer_end_cycles = w32_timer_get_cycles();
                    timer_end_counts = w32_timer_get_counts();

                    {
                        u64 timer_elapsed_cycles = timer_end_cycles - timer_start_cycles;
                        u32 timer_elapsed_Mhz = (timer_elapsed_cycles / 1000000);
                        u64 timer_elapsed_counts = timer_end_counts - timer_start_counts;
                        f32 timer_elapsed_ms = (f32)((1000.0f * (f32)timer_elapsed_counts) / (f32)timer_frequency);
                        f32 timer_frames_per_sec = ((f32)timer_frequency / (f32)timer_elapsed_counts);

#if 0
                        char buf[256];
                        Out_s(buf, sizeof(buf), "Mhz: %u\nms: %f\nfps: %f\n\n",
                              timer_elapsed_Mhz, timer_elapsed_ms, timer_frames_per_sec);
#endif
                    }
                    
                    timer_start_cycles = timer_end_cycles;
                    timer_start_counts = timer_end_counts;
                }
            }
            ReleaseDC(window_handle, global_device_context);
        }
        else{
            // TODO: Error handling ("CreateWindowEx" call failed).
        }
    }
    else{
        // TODO: Error handling ("RegisterClassA" call failed).
    }
    
    return(0);
}
