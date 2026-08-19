#define _CRTDBG_MAP_ALLOC // Memory leak detection
#include <Windows.h>
#include <iostream>

#include <chrono>

#include "Window.h"
#include "Renderer.h"

void UpdateRasterizerDesc(ID3D11Device*& device, ID3D11DeviceContext*& context, bool tesselate);

//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	{
		const UINT WIDTH = 1920;
		const UINT HEIGHT = 1080;

		Window window(hInstance, WIDTH, HEIGHT, nCmdShow);

		//std::unique_ptr<Scene> scene = std::make_unique<Scene>();
		Scene* scene = new Scene();

		Renderer renderer(window, *scene);
		MSG msg = { };

		//float deltatime = renderer.GetDeltatime();
		ComPtr<ID3D11Device> device = renderer.GetDevice();
		ComPtr<ID3D11DeviceContext> immediateContext;
		device->GetImmediateContext(&immediateContext);


		bool shouldTesselate = true;
		bool showWireFrame = false;
		bool shadowOn = true;
		bool particlesOn = true;


		UpdateRasterizerDesc(*device.GetAddressOf(), *immediateContext.GetAddressOf(), showWireFrame);




		// TEMPORARY SPEED VARIABLE
		float movespeed = 3.0f;
		float rotationspeed = 2.0f;

		// Get initial mouse position (center of the screen)
		//ShowCursor(FALSE);
		POINT center;
		center.x = WIDTH / 2;
		center.y = HEIGHT / 2;
		ClientToScreen(window.GetHWND(), &center); // Convert to screen coords
		SetCursorPos(center.x, center.y);

		POINT lastMousePos = center;
		float sensitivity = 0.01f; // Adjust after prefrenc

		float wasPressed = 0.0f;


		//Scene* scene = renderer.GetScene();

		std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
		while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
		{
			auto now = std::chrono::steady_clock::now();
			std::chrono::duration<float> delta = now - lastTime;
			float deltatime = delta.count();
			lastTime = now;

			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (GetKeyState('W') & 0x8000) {
				renderer.GetCamera().MoveForward(movespeed * deltatime);
			}
			if (GetKeyState('S') & 0x8000) {
				renderer.GetCamera().MoveForward(-movespeed * deltatime);
			}
			if (GetKeyState('A') & 0x8000) {
				renderer.GetCamera().MoveRight(-movespeed * deltatime);
			}
			if (GetKeyState('D') & 0x8000) {
				renderer.GetCamera().MoveRight(movespeed * deltatime);
			}
			if (GetKeyState(VK_SPACE) & 0x8000) {
				renderer.GetCamera().MoveUp(movespeed * deltatime);
			}
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				renderer.GetCamera().MoveUp(-movespeed * deltatime);
			}

			if (GetKeyState(VK_UP) & 0x8000)
			{
				renderer.GetCamera().RotateRight(-rotationspeed * deltatime);
			}
			if (GetKeyState(VK_DOWN) & 0x8000)
			{
				renderer.GetCamera().RotateRight(rotationspeed * deltatime);
			}
			if (GetKeyState(VK_LEFT) & 0x8000)
			{
				renderer.GetCamera().RotateUp(-rotationspeed * deltatime);
			}
			if (GetKeyState(VK_RIGHT) & 0x8000)
			{
				renderer.GetCamera().RotateUp(rotationspeed * deltatime);
			}
			if (GetKeyState('R') & 0x8000)
			{
				renderer.GetCamera().ResetUp();
			}

			if (GetKeyState('T') & 0x8000 && wasPressed <= 0)
			{
				shouldTesselate = !shouldTesselate;
				wasPressed = 1.0f;
			}
			if (GetKeyState('X') & 0x8000 && wasPressed <= 0)
			{
				showWireFrame = !showWireFrame;
				UpdateRasterizerDesc(*device.GetAddressOf(), *immediateContext.GetAddressOf(), showWireFrame);
				wasPressed = 1.0f;

			}
			if (GetKeyState('O') & 0x8000 && wasPressed <= 0)
			{
				shadowOn = !shadowOn;
				scene->UpdateNrOfLigthsBuffer(immediateContext.Get(), shadowOn);
				wasPressed = 1.0f;
			}
			if (GetKeyState('P') & 0x8000 && wasPressed <= 0)
			{
				particlesOn = !particlesOn;
				wasPressed = 1.0f;
			}
			wasPressed -= deltatime;


			// Update scene (objects and lights)
			scene->UpdateObjects(immediateContext.Get(), deltatime);
			if (particlesOn)
			{
				renderer.UpdateParticles(*scene);
			}
			renderer.GetCamera().UpdateInternalConstantBuffer(immediateContext.Get());

			// Mouse panning-movement
			/*POINT currentPos;
			GetCursorPos(&currentPos);

			float dx = static_cast<float>(currentPos.x - lastMousePos.x);
			float dy = static_cast<float>(currentPos.y - lastMousePos.y);
			renderer.GetCamera().RotateUp(dx * sensitivity);
			renderer.GetCamera().RotateRight(dy * sensitivity);

			SetCursorPos(center.x, center.y);
			lastMousePos = center;
			*/

			renderer.Render(*scene, shouldTesselate, shadowOn, particlesOn);
		} 

		delete scene;
		scene = nullptr;

	} // <--- All stack objects, vectors, and ComPtrs are fully destroyed here
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks(); // No false positives, this reports 100% accurate leaks

	return 0;
}


void UpdateRasterizerDesc(ID3D11Device*& device, ID3D11DeviceContext*& immediateContext, bool showWireFrame)
{
	D3D11_RASTERIZER_DESC wireFrameDesc = {};
	if (showWireFrame)
		wireFrameDesc.FillMode = D3D11_FILL_WIREFRAME;
	else
		wireFrameDesc.FillMode = D3D11_FILL_SOLID;
	
	wireFrameDesc.CullMode = D3D11_CULL_BACK;
	wireFrameDesc.FrontCounterClockwise = false;

	ID3D11RasterizerState* wireFrameState;
	device->CreateRasterizerState(&wireFrameDesc, &wireFrameState);

	immediateContext->RSSetState(wireFrameState);
}