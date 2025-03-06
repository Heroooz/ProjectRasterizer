#include <Windows.h>
#include <iostream>

#include "Window.h"
#include "Renderer.h"

#define _CRTDBG_MAP_ALLOC // Memory leak detection

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	const UINT WIDTH = 1024;
	const UINT HEIGHT = 576;

	Window window(hInstance, WIDTH, HEIGHT, nCmdShow);
	Renderer renderer(window);
	MSG msg = { };

	// TEMPORARY SPEED VARIABLE
	float movespeed = 0.0005f;

	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (GetKeyState('W') & 0x8000) {
			renderer.GetCamera().MoveForward(movespeed);
		}
		if (GetKeyState('S') & 0x8000) {
			renderer.GetCamera().MoveForward(-movespeed);
		}
		if (GetKeyState('A') & 0x8000) {
			renderer.GetCamera().MoveRight(-movespeed);
		}
		if (GetKeyState('D') & 0x8000) {
			renderer.GetCamera().MoveRight(movespeed);
		}
		if (GetKeyState(VK_SPACE) & 0x8000) {
			renderer.GetCamera().MoveUp(movespeed);
		}
		if (GetKeyState(VK_CONTROL) & 0x8000) {
			renderer.GetCamera().MoveUp(-movespeed);
		}
		if (GetKeyState('Q') & 0x8000) {
			renderer.GetCamera().RotateUp(-movespeed);
		}
		if (GetKeyState('E') & 0x8000) {
			renderer.GetCamera().RotateUp(movespeed);
		}

		renderer.Render();
	}

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks();

	return 0;
}
