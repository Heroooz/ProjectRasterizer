#include "Window.h"
#include "WindowHelper.h"
#include <iostream>

Window::Window(HINSTANCE hInstance, UINT width, UINT height, int nCmdShow)
    : hInstance(hInstance), width(width), height(height), nCmdShow(nCmdShow), hwnd(nullptr) {

    if (!Window::Initialize()) {
        std::cerr << "Failed to setup window!" << std::endl;
    }
}

Window::~Window() {}

bool Window::Initialize() {

    if (!SetupWindow(hInstance, width, height, nCmdShow, hwnd))
    {
        std::cerr << "Failed to setup window!" << std::endl;
        return false;
    }
    return true;
}

HWND Window::GetHWND() const {
    return hwnd;
}

UINT Window::GetWidth() const
{
    return width;
}

UINT Window::GetHeight() const
{
    return height;
}
