#pragma once

#include <windows.h>
#include <string>
#include <functional>

namespace Aetheria {

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    bool processMessages();
    bool shouldClose() const { return closeRequested; }
    void close() { closeRequested = true; }

    HWND getHandle() const { return hwnd; }
    HINSTANCE getInstance() const { return hinstance; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    float getAspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }

    // Input
    bool isKeyDown(int vKey) const {
        int vk = (vKey >= 'a' && vKey <= 'z') ? (vKey - 'a' + 'A') : vKey;
        return keys[vk & 0xFF];
    }
    bool isKeyPressed(int vKey) const {
        int vk = (vKey >= 'a' && vKey <= 'z') ? (vKey - 'a' + 'A') : vKey;
        return keysPressed[vk & 0xFF];
    }
    bool isMouseButtonDown(int button) const { return mouseButtons[button & 3]; }
    bool isMouseButtonPressed(int button) const { return mouseButtonsPressed[button & 3]; }
    
    void resetPressedKeys() {
        for (int i = 0; i < 256; ++i) keysPressed[i] = false;
        for (int i = 0; i < 4; ++i) mouseButtonsPressed[i] = false;
        mouseDeltaX = 0;
        mouseDeltaY = 0;
        mouseWheelDelta = 0;
    }

    int getMouseDeltaX() const { return mouseDeltaX; }
    int getMouseDeltaY() const { return mouseDeltaY; }
    int getMouseWheelDelta() const { return mouseWheelDelta; }
    int getMouseX() const { return mouseX; }
    int getMouseY() const { return mouseY; }

    void setCursorLocked(bool locked);
    bool isCursorLocked() const { return cursorLocked; }

    void setResizeCallback(std::function<void(int, int)> callback) { resizeCallback = callback; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwnd = nullptr;
    HINSTANCE hinstance = nullptr;
    int width = 1280;
    int height = 720;
    bool closeRequested = false;
    bool cursorLocked = false;

    bool keys[256] = {};
    bool keysPressed[256] = {};
    bool mouseButtons[4] = {};
    bool mouseButtonsPressed[4] = {};

    int mouseX = 0;
    int mouseY = 0;
    int mouseDeltaX = 0;
    int mouseDeltaY = 0;
    int mouseWheelDelta = 0;

    std::function<void(int, int)> resizeCallback;
};

} // namespace Aetheria
