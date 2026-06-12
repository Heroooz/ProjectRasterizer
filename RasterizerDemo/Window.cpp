#include "Window.h"
#include "WindowHelper.h"
#include <iostream>
#include <io.h>


Window::Window(HINSTANCE hInstance, UINT width, UINT height, int nCmdShow)
    : hInstance(hInstance), width(width), height(height), nCmdShow(nCmdShow), hwnd(nullptr) {

    if (AttachConsole(ATTACH_PARENT_PROCESS) == 0)
    {
        ::AllocConsole();
    }
    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);

    std::ios::sync_with_stdio(true);
    std::cout.clear();
    std::cerr.clear();

    SetConsoleTitle(L"Game Console");


    if (!Window::Initialize()) {
        std::cerr << "Failed to setup window!" << std::endl;
        throw std::runtime_error("Failed to setup window!");
    }
}

Window::~Window() 
{
    if (hwnd)
    {
        DestroyWindow(hwnd);
    }
    UnregisterClass(L"Test Window Class", hInstance);
}

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
