


/*
  TODO:

  [x] Fullscreen mode.
  [x] Initialize DirectSound and output Square/Sine wave to the sound buffer.
  [] Gamepad input (deal with DEADZONEs), and test the input.
  [] Raw input (support for multiple keyboars).
  [x] Mouse position.
  [] Mouse input.
  [] WM_ACTIVATEAPP (handle case when application is not active).
  [] Timing (QueryPerformanceCounter, RDTSC).
  [] Proper OpenGL initialization.
  [] Stuby about true type font.
  [] Figure out what's going on in stb_truetype.h
  [] Implement corrent text renderer (with kerning table).
  ...
  
  
 */



#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <gl/gl.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "ext/stb_truetype.h"

#define global static
#define internal static
#define local_persist static

#define const
#define Pi32 3.14159265358f
#define Out(s) OutputDebugStringA(s);
#define Out_s(buf, buf_size, format, ...)           \
    sprintf_s(buf, buf_size, format, __VA_ARGS__);  \
    Out(buf);

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

#include "ced.c"

typedef struct W32_WindowDimension W32_WindowDimension;
struct W32_WindowDimension{
    s32 width;
    s32 height;
};

typedef struct W32_Bitmap W32_Bitmap;
struct W32_Bitmap{
    BITMAPINFO info;
    void *memory;
    s32 width;
    s32 height;
    u32 pitch;
};

typedef struct SoundOutput SoundOutput;
struct SoundOutput{
    s32 samples_per_second;
    s32 bytes_per_sample;
    s32 buffer_size;
    s32 tone_volume;
    s32 tone_Hz;
    s32 wave_period;
    u32 running_sample_index;
};

global s32 global_running;
global W32_Bitmap global_offscreen_buffer;
global LPDIRECTSOUNDBUFFER global_secondary_sound_buffer;

internal W32_WindowDimension w32_get_window_dimension(HWND window_handle);
internal void w32_load_xinput(void);
internal void w32_init_opengl(HWND window_handle);
internal void w32_toggle_fullscreen(HWND window_handle);
internal V2 w32_get_mouse_pos(HWND window_handle);
internal void w32_init_dsound(HWND window_handle, s32 samples_per_second, s32 bytes_per_sample);
internal void w32_fill_sound_buffer(SoundOutput *sound_output, s32 byte_to_lock, s32 bytes_to_write);



internal void w32_fill_sound_buffer(SoundOutput *sound_output, s32 byte_to_lock, s32 bytes_to_write){
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

    HDC device_context = GetDC(window_handle);

    if((pixel_format = ChoosePixelFormat(device_context, &pfd)) != 0){
        if(SetPixelFormat(device_context, pixel_format, &pfd) == TRUE){
            HGLRC opengl_dummy_context = wglCreateContext(device_context);
        
            if(wglMakeCurrent(device_context, opengl_dummy_context)){
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
    ReleaseDC(window_handle, device_context);
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

internal void w32_load_xinput(void){
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

LRESULT CALLBACK w32_main_window_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param){
    LRESULT result = 0;
    local_persist fullscreen_mode = 0;
    
    switch(message){
        case(WM_ACTIVATEAPP):{
            Out("App was activated!\n");
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

            result = DefWindowProc(window_handle, message, w_param, l_param);
        }break;
        case(WM_LBUTTONUP):{
            w32_toggle_fullscreen(window_handle);
        }break;
        case(WM_SIZE):{
            // TODO: Handle somehow.
        };
        default:{
            result = DefWindowProc(window_handle, message, w_param, l_param);
        }break;
    }
    return(result);
}


INT WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE prev_instance,
                   PSTR cmd_line,
                   int show_code){
    w32_load_xinput();
    
    
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
            SoundOutput sound_output = {0};
            {
                sound_output.samples_per_second = 48000;
                sound_output.bytes_per_sample = sizeof(s16) * 2;
                sound_output.buffer_size = (sound_output.samples_per_second * sound_output.bytes_per_sample);
                sound_output.tone_volume = 400;
                sound_output.tone_Hz = 256; // middle c.
                sound_output.running_sample_index = 0;
                sound_output.wave_period = (sound_output.samples_per_second / sound_output.tone_Hz);
            }
            w32_init_dsound(window_handle, sound_output.samples_per_second, sound_output.bytes_per_sample);
            w32_fill_sound_buffer(&sound_output, 0, sound_output.buffer_size);
            
            HDC device_context = GetDC(window_handle);

            global_running = 1;
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
                    game_update_and_render();
                    SwapBuffers(device_context);
                }
            }
            ReleaseDC(window_handle, device_context);
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
