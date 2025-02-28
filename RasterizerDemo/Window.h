#pragma once
#include <Windows.h>

class Window {
public:
    Window(HINSTANCE hInstance, UINT width, UINT height, int nCmdShow);
    ~Window();

    bool Initialize();
    HWND GetHWND() const;
    UINT GetWidth() const;
    UINT GetHeight() const;

private:
    HINSTANCE hInstance;
    UINT width;
    UINT height;
    int nCmdShow;
    HWND hwnd;
};