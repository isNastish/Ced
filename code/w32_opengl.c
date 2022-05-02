

internal void *w32_load_opengl_proc(char *proc_name){
    void *result = 0;

    return(result);
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
        MessageBox(0, (char *)glGetString(GL_VERSION), "OpenGL version", 0);
        result = 1;
    }
    else{
        // TODO: error handling.
    }

    return(result);
}


