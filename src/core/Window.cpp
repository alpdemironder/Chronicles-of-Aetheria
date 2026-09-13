#include "Window.hpp"

namespace Aetheria {

Window* g_CurrentWindow = nullptr;

Window::Window(const std::string& title, int width, int height)
    : width(width), height(height) {
    g_CurrentWindow = this;
    hinstance = GetModuleHandle(nullptr);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hinstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "AetheriaVulkanWindowClass";

    RegisterClassEx(&wc);

    RECT wr = { 0, 0, width, height };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, hinstance, nullptr
    );

    // Register Raw Input for smooth 3D camera look
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic desktop controls
    rid.usUsage = 0x02;     // Mouse
    rid.dwFlags = 0;
    rid.hwndTarget = hwnd;
    RegisterRawInputDevices(&rid, 1, sizeof(rid));
}

Window::~Window() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = nullptr;
    }
    UnregisterClass("AetheriaVulkanWindowClass", hinstance);
}

bool Window::processMessages() {
    resetPressedKeys();

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            closeRequested = true;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (cursorLocked && GetForegroundWindow() == hwnd) {
        RECT rect;
        GetClientRect(hwnd, &rect);
        POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
        ClientToScreen(hwnd, &center);
        SetCursorPos(center.x, center.y);
    }

    return !closeRequested;
}

void Window::setCursorLocked(bool locked) {
    if (cursorLocked == locked) return;
    cursorLocked = locked;
    if (locked) {
        while (ShowCursor(FALSE) >= 0);
        RECT rect;
        GetClientRect(hwnd, &rect);
        POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
        ClientToScreen(hwnd, &center);
        SetCursorPos(center.x, center.y);
    } else {
        while (ShowCursor(TRUE) < 0);
    }
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!g_CurrentWindow) return DefWindowProc(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_CLOSE:
        g_CurrentWindow->closeRequested = true;
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_SIZE: {
        int newWidth = LOWORD(lParam);
        int newHeight = HIWORD(lParam);
        if (newWidth > 0 && newHeight > 0) {
            g_CurrentWindow->width = newWidth;
            g_CurrentWindow->height = newHeight;
            if (g_CurrentWindow->resizeCallback) {
                g_CurrentWindow->resizeCallback(newWidth, newHeight);
            }
        }
        return 0;
    }

    case WM_KILLFOCUS: {
        for (int i = 0; i < 256; ++i) {
            g_CurrentWindow->keys[i] = false;
            g_CurrentWindow->keysPressed[i] = false;
        }
        for (int i = 0; i < 4; ++i) {
            g_CurrentWindow->mouseButtons[i] = false;
            g_CurrentWindow->mouseButtonsPressed[i] = false;
        }
        return 0;
    }

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN: {
        int vk = static_cast<int>(wParam);
        if (vk < 256) {
            bool wasDown = (lParam & (1 << 30)) != 0;
            if (!wasDown || !g_CurrentWindow->keys[vk]) {
                g_CurrentWindow->keysPressed[vk] = true;
            }
            g_CurrentWindow->keys[vk] = true;
        }
        return 0;
    }

    case WM_KEYUP:
    case WM_SYSKEYUP: {
        int vk = static_cast<int>(wParam);
        if (vk < 256) {
            g_CurrentWindow->keys[vk] = false;
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        g_CurrentWindow->mouseX = LOWORD(lParam);
        g_CurrentWindow->mouseY = HIWORD(lParam);
        return 0;
    }

    case WM_INPUT: {
        UINT dwSize = sizeof(RAWINPUT);
        static BYTE lpb[sizeof(RAWINPUT)];
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
        RAWINPUT* raw = (RAWINPUT*)lpb;
        if (raw->header.dwType == RIM_TYPEMOUSE) {
            if (g_CurrentWindow->cursorLocked) {
                g_CurrentWindow->mouseDeltaX += raw->data.mouse.lLastX;
                g_CurrentWindow->mouseDeltaY += raw->data.mouse.lLastY;
            }
        }
        return 0;
    }

    case WM_LBUTTONDOWN:
        g_CurrentWindow->mouseButtons[0] = true;
        g_CurrentWindow->mouseButtonsPressed[0] = true;
        return 0;
    case WM_LBUTTONUP:
        g_CurrentWindow->mouseButtons[0] = false;
        return 0;

    case WM_RBUTTONDOWN:
        g_CurrentWindow->mouseButtons[1] = true;
        g_CurrentWindow->mouseButtonsPressed[1] = true;
        return 0;
    case WM_RBUTTONUP:
        g_CurrentWindow->mouseButtons[1] = false;
        return 0;

    case WM_MBUTTONDOWN:
        g_CurrentWindow->mouseButtons[2] = true;
        g_CurrentWindow->mouseButtonsPressed[2] = true;
        return 0;
    case WM_MBUTTONUP:
        g_CurrentWindow->mouseButtons[2] = false;
        return 0;

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        g_CurrentWindow->mouseWheelDelta += delta / WHEEL_DELTA;
        return 0;
    }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

} // namespace Aetheria
