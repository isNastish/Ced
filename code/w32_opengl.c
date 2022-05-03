

internal void *w32_load_opengl_procedure(char *proc_name){
    // NOTE: proc_name can be an OpenGL function or a platform-specific WGL function.
#if defined(WIN32)
    void *proc = (void *)wglGetProcAddress(proc_name);
#elif UNIX
    void *proc = (void *)glXGetProcAddress(proc_name);
#else // OS X
    // NOTE: Metal
#endif
    if(!proc ||
       (proc == (void *)0x1) ||
       (proc == (void *)0x2) ||
       (proc == (void *)0x3) ||
       (proc == (void *)-1)){
        return(0);
    }
    return(proc);
}

internal void w32_load_wgl_functions(HINSTANCE instance){
    // NOTE: functions to load:
        
    // wglChoosePixelFormatARB
    // wglCreateContextAttribsARB
    // wglMakeContextCurrentARB
    // wglSwapIntervalEXT

    // TODO: 
#if 0
    wglChoosePixelFormatARB = ;
    wglCreateContextAttribsARB = ;
    wglMakeContextCurrentARB = ;
    wglSwapIntervalEXT = ;
#endif
}

internal b32 w32_init_opengl(HWND window){
    b32 result = 0;

    s32 pixel_format = 0;
    PIXELFORMATDESCRIPTOR pfd = {0};
    
    {
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA; // The kind of framebuffer, RGBA or palette.
        pfd.cColorBits = 32; // Color depth of the framebuffer.
        pfd.cDepthBits = 24; // Number of bits for the depthbuffer.
        pfd.cStencilBits = 8; // Number of bits for the stencilbuffer.
        pfd.iLayerType = PFD_MAIN_PLANE;
    }

    
    global_device_context = GetDC(window);

    pixel_format = ChoosePixelFormat(global_device_context, &pfd);
    SetPixelFormat(global_device_context,
                   pixel_format,
                   &pfd);
    
    global_opengl_rendering_context = wglCreateContext(global_device_context);
    if(wglMakeCurrent(global_device_context,
                      global_opengl_rendering_context)){
        char opengl_version_buf[256];
        sprintf_s(opengl_version_buf,
                  sizeof(opengl_version_buf),
                  "OpenGL version: %s.\n",
                  (char *)glGetString(GL_VERSION));
        OutputDebugStringA(opengl_version_buf);
        
        result = 1;
    }
    else{
        // TODO: error handling.
    }

    // NOTE: Should I call ReleaseDC()?
    return(result);
}

/*
alphabet_quote:
{
    Here is some text for typing.
    It's very simple. But I have to find a way how to parse it using C programming language.

};

alphabet_quote:
{
    Here is some text for typing.
    It's very simple. But I have to find a way how to parse it using C programming language.

};

alphabet_quote:
{
    Here is some text for typing.
    It's very simple. But I have to find a way how to parse it using C programming language.

};

alphanumerical_quote:
{
    This quote includes number and letters. Numbers like this and quotes: 
};

numerical_quote:
{
    1245 4 5 3344 89887 2890078 344 8932
};

*/
