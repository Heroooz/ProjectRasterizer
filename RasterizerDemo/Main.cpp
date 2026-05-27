#include <Windows.h>
#include <iostream>

#include "Window.h"
#include "Renderer.h"

#define _CRTDBG_MAP_ALLOC // Memory leak detection

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	const UINT WIDTH = 1920;
	const UINT HEIGHT = 1080;

	Window window(hInstance, WIDTH, HEIGHT, nCmdShow);
	Renderer renderer(window);
	MSG msg = { };

	// TEMPORARY SPEED VARIABLE
	float movespeed = 0.003f;
	float rotationspeed = 0.0005f;

	// Get initial mouse position (center of the screen)
	//ShowCursor(FALSE);
	POINT center;
	center.x = WIDTH / 2;
	center.y = HEIGHT / 2;
	ClientToScreen(window.GetHWND(), &center); // Convert to screen coords
	SetCursorPos(center.x, center.y);

	POINT lastMousePos = center;
	float sensitivity = 0.01f; // Adjust after prefrence

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

		if (GetKeyState(VK_UP) & 0x8000) {
			renderer.GetCamera().RotateRight(-rotationspeed);
		}
		if (GetKeyState(VK_DOWN) & 0x8000)
		{
			renderer.GetCamera().RotateRight(rotationspeed);
		}
		if (GetKeyState(VK_LEFT) & 0x8000)
		{
			renderer.GetCamera().RotateUp(-rotationspeed);
		}
		if (GetKeyState(VK_RIGHT) & 0x8000)
		{
			renderer.GetCamera().RotateUp(rotationspeed);
		}

		// Mouse panning-movement
		//POINT currentPos;
		//GetCursorPos(&currentPos);

		//float dx = static_cast<float>(currentPos.x - lastMousePos.x);
		//float dy = static_cast<float>(currentPos.y - lastMousePos.y);
		//renderer.GetCamera().RotateUp(dx * sensitivity);
		//renderer.GetCamera().RotateRight(dy * sensitivity);

		//SetCursorPos(center.x, center.y);
		//lastMousePos = center;

		renderer.Render();
	}

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks();

	return 0;
}
