#pragma once

#include "GLHeaders.hpp"

namespace Aetheria {

class Window;

class GLContext {
public:
    GLContext(Window* window);
    ~GLContext();

    bool isInitialized() const { return initialized; }
    void swapBuffers();
    void setVSync(bool enabled);
    void setWireframe(bool enabled);
    void resize(int width, int height);

private:
    Window* window = nullptr;
    HDC hdc = nullptr;
    HGLRC hglrc = nullptr;
    bool initialized = false;
};

} // namespace Aetheria
