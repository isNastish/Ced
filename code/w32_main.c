

#include <stdint.h>
#include <windows.h>
#include <xinput.h>
#include <gl/gl.h>

#define global static
#define internal static
#define local_persist static

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

// NOTE: Structs:
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

typedef struct GamepadInput GamepadInput;
struct GamepadInput{
    u8 is_connected;
    struct
    {
        b32 dpad_up;
        b32 dpad_down;
        b32 dpad_left;
        b32 dpad_right;
        
        b32 start;
        b32 back;
        
        b32 left_thumb;
        b32 right_thumb;
        
        b32 left_shoulder;
        b32 right_shoulder;
        
        b32 A_button;
        b32 B_button;
        b32 X_button;
        b32 Y_button;
        
        b32 left_trigger;
        b32 right_trigger;
        
        s16 left_thumbstick_x;
        s16 left_thumbstick_y;
        s16 right_thumbstick_x;
        s16 right_thumbstick_y;
    };
};

typedef struct GameState GameState;
struct GameState{
    void *game_memory; // NOTE: not used for now.
    s32 game_is_running;
    W32_Bitmap offscreen_bitmap;
    GamepadInput gamepad_input[4];
};

// NOTE: Function definitions:
internal W32_WindowDimension w32_get_window_dimension(HWND window_handle);
internal void w32_resize_dib_section(W32_Bitmap *bitmap, s32 window_width, s32 window_height);
internal void w32_display_offscreen_buffer_in_window(W32_Bitmap *offscreen_bitmap,
                                                     HDC device_context,
                                                     s32 window_width,
                                                     s32 window_height);
internal void w32_load_xinput(void);
internal void w32_init_opengl(HWND window_handle);

// NOTE: Global variables:
global GameState global_game_state;

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
                char opengl_version_buf[128];
                sprintf_s(opengl_version_buf,
                          sizeof(opengl_version_buf),
                          "OpenGL version: %s.\n",
                          (char *)glGetString(GL_VERSION));
                OutputDebugStringA(opengl_version_buf);
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

internal void w32_resize_dib_section(W32_Bitmap *bitmap, s32 window_width, s32 window_height){
    if(bitmap->memory){
        VirtualFree(bitmap->memory, 0, MEM_RELEASE);
    }
    
    bitmap->width = window_width;
    bitmap->height = window_height;
    
    bitmap->info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap->info.bmiHeader.biWidth = bitmap->width;
    bitmap->info.bmiHeader.biHeight = -bitmap->height; // NOTE: top-down dib (origin in the upper-left corner).
    bitmap->info.bmiHeader.biPlanes = 1;
    bitmap->info.bmiHeader.biBitCount = 32;
    bitmap->info.bmiHeader.biCompression = BI_RGB;

    u32 bpp = 4; // NOTE: bytes per pixel.
    u32 bitmap_size = (bitmap->width * bitmap->height) * bpp;

    bitmap->memory = VirtualAlloc(0, bitmap_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    bitmap->pitch = (bitmap->width * bpp);
}

internal void w32_display_offscreen_buffer_in_window(W32_Bitmap *offscreen_bitmap,
                                                     HDC device_context,
                                                     s32 window_width,
                                                     s32 window_height){
    s32 scanlines_count = StretchDIBits(device_context,
                                        0, 0,
                                        window_width,
                                        window_height,
                                        0, 0,
                                        offscreen_bitmap->width,
                                        offscreen_bitmap->height,
                                        offscreen_bitmap->memory,
                                        &offscreen_bitmap->info,
                                        DIB_RGB_COLORS,
                                        SRCCOPY);
    if(!scanlines_count){
        // TODO: error handling.
    }
}

LRESULT CALLBACK w32_main_window_proc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param){
    LRESULT result = 0;
    
    switch(message){
        case(WM_CREATE):{ 
            w32_init_opengl(window_handle);
        }break;
        case(WM_CLOSE):{
            global_game_state.game_is_running = 0;
        };
        case(WM_DESTROY ):{
            PostQuitMessage(0);
        };
        case(WM_SYSKEYDOWN):
        case(WM_SYSKEYUP):
        case(WM_KEYDOWN):
        case(WM_KEYUP):{
            s32 vk_code = w_param;
            s32 was_down = ((l_param & (1 << 30)) != 0);
            s32 is_down = ((l_param & (1 << 31)) == 0);

            if(was_down != is_down){
                if(vk_code == VK_SHIFT){
                    OutputDebugStringA("SHIFT\n");
                }
                else if(vk_code == VK_BACK){
                    OutputDebugStringA("BACK\n");
                }
                else if(vk_code == VK_SPACE){
                    OutputDebugStringA("SPACE\n");
                }
                else if((vk_code == VK_F4) && (l_param & (1 << 29))){
                    OutputDebugStringA("ALT+F4\n");
                }
                else if(vk_code == 'A'){
                    OutputDebugStringA("A\n");
                }
                else if(vk_code == 'W'){
                    OutputDebugStringA("W\n");
                }
                else if(vk_code == 'S'){
                    OutputDebugStringA("S\n");
                }
                else if(vk_code == 'D'){
                    OutputDebugStringA("D\n");
                }
                else if(vk_code == 'Q'){
                    OutputDebugStringA("Q\n");
                }
                else if(vk_code == 'E'){
                    OutputDebugStringA("E\n");
                }
            }
            result = DefWindowProc(window_handle, message, w_param, l_param);
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
    
    if(!RegisterClassA(&window_class)){
        goto quit;
    }
    
    HWND window_handle = CreateWindowEx(0,
                                        window_class.lpszClassName, 
                                        "Ced",
                                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        0, 0,
                                        instance,
                                        0);
    
    if(!window_handle){
        goto quit;
    }

    global_game_state.game_is_running = 1;
    while(global_game_state.game_is_running){
        
        MSG message;
        
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        // XInput:
        s32 gamepad_id;
        for(gamepad_id = 0;
            gamepad_id < XUSER_MAX_COUNT;
            ++gamepad_id){
            XINPUT_STATE controller_state = {0};

            if(XInputGetState(gamepad_id, &controller_state) == ERROR_SUCCESS){
                // NOTE: Controller is connected.
                XINPUT_GAMEPAD *pad = &controller_state.Gamepad;

                global_game_state.gamepad_input[gamepad_id].dpad_up = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                global_game_state.gamepad_input[gamepad_id].dpad_down = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                global_game_state.gamepad_input[gamepad_id].dpad_left = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                global_game_state.gamepad_input[gamepad_id].dpad_right = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                
                global_game_state.gamepad_input[gamepad_id].start = (pad->wButtons & XINPUT_GAMEPAD_START);
                global_game_state.gamepad_input[gamepad_id].back = (pad->wButtons & XINPUT_GAMEPAD_BACK);
                
                global_game_state.gamepad_input[gamepad_id].left_thumb = (pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
                global_game_state.gamepad_input[gamepad_id].right_thumb = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
                
                global_game_state.gamepad_input[gamepad_id].left_shoulder = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                global_game_state.gamepad_input[gamepad_id].right_shoulder = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);

                global_game_state.gamepad_input[gamepad_id].A_button = (pad->wButtons & XINPUT_GAMEPAD_A);
                global_game_state.gamepad_input[gamepad_id].B_button = (pad->wButtons & XINPUT_GAMEPAD_B);
                global_game_state.gamepad_input[gamepad_id].X_button = (pad->wButtons & XINPUT_GAMEPAD_X);
                global_game_state.gamepad_input[gamepad_id].Y_button = (pad->wButtons & XINPUT_GAMEPAD_Y);

                global_game_state.gamepad_input[gamepad_id].left_trigger = pad->bLeftTrigger;
                global_game_state.gamepad_input[gamepad_id].right_trigger = pad->bRightTrigger;

                global_game_state.gamepad_input[gamepad_id].left_thumbstick_x = pad->sThumbLX;
                global_game_state.gamepad_input[gamepad_id].left_thumbstick_y = pad->sThumbLY;
                global_game_state.gamepad_input[gamepad_id].right_thumbstick_x = pad->sThumbRX;
                global_game_state.gamepad_input[gamepad_id].right_thumbstick_y = pad->sThumbRY;
            }
            else{
                // NOTE: Controller is not connected.
            }

            // NOTE: Controller test.
            XINPUT_VIBRATION xinput_vibration = {0};
            {
                xinput_vibration.wLeftMotorSpeed = 65535;
                xinput_vibration.wRightMotorSpeed = 65535;
            }
            XInputSetState(gamepad_id, &xinput_vibration);
        }

        
        // OpenGL
        glClearColor(180.0/255.0, 180.0/255.0, 67.0/255.0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        HDC device_context = GetDC(window_handle);
        SwapBuffers(device_context);
        ReleaseDC(window_handle, device_context);
    }
quit:;
    return(0);
}
