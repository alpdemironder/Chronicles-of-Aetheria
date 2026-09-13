#include "GLContext.hpp"
#include "../core/Window.hpp"
#include <iostream>

namespace Aetheria {

GLContext::GLContext(Window* window) : window(window) {
    HWND hwnd = window->getHandle();
    hdc = GetDC(hwnd);
    if (!hdc) {
        std::cerr << "Failed to GetDC for OpenGL!" << std::endl;
        return;
    }

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(hdc, &pfd);
    if (format == 0 || !SetPixelFormat(hdc, format, &pfd)) {
        std::cerr << "Failed to set OpenGL pixel format!" << std::endl;
        return;
    }

    HGLRC tempContext = wglCreateContext(hdc);
    if (!tempContext || !wglMakeCurrent(hdc, tempContext)) {
        std::cerr << "Failed to create initial OpenGL context!" << std::endl;
        return;
    }

    // Load modern OpenGL function pointers
    if (!initGLFunctions()) {
        std::cerr << "Failed to initialize OpenGL extensions!" << std::endl;
        hglrc = tempContext;
    } else {
        // Attempt creating 3.3 Core profile context
        if (wglCreateContextAttribsARB) {
            int attribs[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };
            HGLRC coreContext = wglCreateContextAttribsARB(hdc, nullptr, attribs);
            if (coreContext) {
                wglMakeCurrent(nullptr, nullptr);
                wglDeleteContext(tempContext);
                wglMakeCurrent(hdc, coreContext);
                hglrc = coreContext;
                std::cout << "OpenGL 3.3 Core context created successfully!" << std::endl;
            } else {
                hglrc = tempContext;
            }
        } else {
            hglrc = tempContext;
        }
    }

    // Initial GL states
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    setVSync(true);

    RECT r;
    GetClientRect(hwnd, &r);
    glViewport(0, 0, r.right - r.left, r.bottom - r.top);

    initialized = true;
    std::cout << "GLContext initialized! Renderer: " << glGetString(GL_RENDERER)
              << " | OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
}

GLContext::~GLContext() {
    if (hglrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(hglrc);
        hglrc = nullptr;
    }
    if (hdc && window) {
        ReleaseDC(window->getHandle(), hdc);
        hdc = nullptr;
    }
}

void GLContext::swapBuffers() {
    if (hdc) {
        SwapBuffers(hdc);
    }
}

void GLContext::setVSync(bool enabled) {
    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(enabled ? 1 : 0);
    }
}

void GLContext::setWireframe(bool enabled) {
    glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
}

void GLContext::resize(int width, int height) {
    if (width > 0 && height > 0) {
        glViewport(0, 0, width, height);
    }
}

} // namespace Aetheria
