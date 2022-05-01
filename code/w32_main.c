

#include <windows.h>
#include <stdint.h>

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

global s8 global_running;
global HINSTANCE global_instance_handle;

LRESULT w32_window_proc(HWND window,
                        UINT msg,
                        WPARAM w_param,
                        LPARAM l_param){
    LRESULT result = 0;
    
    switch(msg){
        case(WM_CLOSE):
        case(WM_DESTROY ):
        case(WM_QUIT):{
            global_running = 0;
        }break;
        default:{
            result = DefWindowProc(window, msg, w_param, l_param);
        }break;
    }
    
    return(result);
}


INT WINAPI WinMain(HINSTANCE instance,
                   HINSTANCE prev_instance,
                   PSTR cmd_line,
                   int show_code){

    global_instance_handle = instance;
    
    WNDCLASSA window_class = {0};
    
    {
        window_class.style = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc = w32_window_proc;
        window_class.hInstance = global_instance_handle;
        //window_class.hIcon = ;
        //window_class.hCursor = ;
        window_class.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
        window_class.lpszClassName = "AppWindowClass";
    }


    if(!RegisterClassA(&window_class)){
        // TODO: error handling.
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
                                        global_instance_handle,
                                        0);

    if(!window_handle){
        // TODO: error handling.
        goto quit;
        
    }


    global_running = 1;
    while(global_running){
        
        MSG message;
        while(PeekMessage(&message, 0, 0, 0, PM_REMOVE)){

            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }

quit:;
    
    return(0);
}
