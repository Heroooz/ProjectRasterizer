#include <Windows.h>
#include <iostream>
#include <d3d11.h>
#include <DirectXMath.h>
#include <chrono>

#include "WindowHelper.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"

DirectX::XMMATRIX CreateWorldMatrix(float angle) {

	//eventuellt lägga till en scaleMatrix
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(0.0f, 0.0f, -1.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationY(angle);
	return XMMatrixTranspose(DirectX::XMMatrixMultiply(translationMatrix, rotationMatrix));
}

DirectX::XMMATRIX CreateViewMatrix() {
	DirectX::XMVECTOR eyePosition = DirectX::XMVectorSet(0.0f, 0.0f, -3.0f, 1.0f);
	DirectX::XMVECTOR focusPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	return DirectX::XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
}

DirectX::XMMATRIX CreateProjectionMatrix(const float fovAngle, const float aspectRatio, const float nearZ, const float farZ) {
	return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fovAngle), aspectRatio, nearZ, farZ);
}

bool CreateVSConstantBuffer(ID3D11Device* device, ID3D11Buffer*& vsConstantBuffer, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT) {
	D3D11_BUFFER_DESC vertexShaderConstBufferDesc;
	vertexShaderConstBufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT4X4) * 2;
	vertexShaderConstBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexShaderConstBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vertexShaderConstBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexShaderConstBufferDesc.MiscFlags = 0;
	vertexShaderConstBufferDesc.StructureByteStride = 0;

	DirectX::XMMATRIX worldMatrix = CreateWorldMatrix(rotation);
	DirectX::XMFLOAT4X4 worldMatrixFloat4x4;
	DirectX::XMStoreFloat4x4(&worldMatrixFloat4x4, worldMatrix);

	DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixMultiplyTranspose(CreateViewMatrix(), CreateProjectionMatrix(59.0f, static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f));
	DirectX::XMFLOAT4X4 viewMatrixFloat4x4;
	DirectX::XMStoreFloat4x4(&viewMatrixFloat4x4, viewMatrix);

	matrixArr[0] = worldMatrixFloat4x4;
	matrixArr[1] = viewMatrixFloat4x4;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = matrixArr;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&vertexShaderConstBufferDesc, &data, &vsConstantBuffer);
	return !FAILED(hr);
}

bool CreatePointLight(ID3D11Device* device, ID3D11Buffer*& lightConstantBuffer) {

	struct lightStruct
	{
		DirectX::XMFLOAT4 lightPosition = { 0.0f, 0.5f, -5.0f, 0.0f };
		DirectX::XMFLOAT4 lightColour = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4 cameraPosition = { 0.0f, 0.0f, -3.0f, 1.0f };
		float lightIntensity = 0.01f;
		float shininess = 100.0f;
		char padding[8];
	};

	lightStruct light;
	D3D11_BUFFER_DESC lightConstantBufferDesc;
	lightConstantBufferDesc.ByteWidth = sizeof(lightStruct);
	lightConstantBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	lightConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightConstantBufferDesc.CPUAccessFlags = 0;
	lightConstantBufferDesc.MiscFlags = 0;
	lightConstantBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA lightData;
	lightData.pSysMem = &light;
	lightData.SysMemPitch = 0;
	lightData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&lightConstantBufferDesc, &lightData, &lightConstantBuffer);
	return !FAILED(hr);
}

void Render(ID3D11DeviceContext* immediateContext, ID3D11RenderTargetView* rtv,
			ID3D11DepthStencilView* dsView, D3D11_VIEWPORT& viewport, ID3D11VertexShader* vShader,
			ID3D11PixelShader* pShader, ID3D11InputLayout* inputLayout, ID3D11Buffer* vertexBuffer, 
			ID3D11Buffer* vsConstantBuffer, ID3D11Buffer* psConstantBuffer, 
			ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler)
{
	float clearColour[4] = { 0, 0, 0, 0 };
	immediateContext->ClearRenderTargetView(rtv, clearColour);
	immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	immediateContext->IASetInputLayout(inputLayout);
	immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	immediateContext->VSSetShader(vShader, nullptr, 0);
	immediateContext->VSSetConstantBuffers(0, 1, &vsConstantBuffer);
	immediateContext->RSSetViewports(1, &viewport);
	immediateContext->PSSetShader(pShader, nullptr, 0);
	immediateContext->PSSetShaderResources(0, 1, &srv);
	immediateContext->PSSetSamplers(0, 1, &sampler);
	immediateContext->PSSetConstantBuffers(0, 1, &psConstantBuffer);
	immediateContext->OMSetRenderTargets(1, &rtv, dsView);

	immediateContext->Draw(4, 0);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	const UINT WIDTH = 1024;
	const UINT HEIGHT = 576;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		std::cerr << "Failed to setup window!" << std::endl;
		return -1;
	}

	ID3D11Device* device;
	ID3D11DeviceContext* immediateContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* dsTexture;
	ID3D11DepthStencilView* dsView;
	D3D11_VIEWPORT viewport;
	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;
	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* vsConstantBuffer;	
	ID3D11Buffer* lightConstantBuffer;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* samplerState;

	if (!SetupD3D11(WIDTH, HEIGHT, window, device, immediateContext, swapChain, rtv, dsTexture, dsView, viewport))
	{
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return -1;
	}

	if (!SetupPipeline(device, vertexBuffer, vShader, pShader, inputLayout, texture, srv, samplerState))
	{
		std::cerr << "Failed to setup pipeline!" << std::endl;
		return -1;
	}

	float rotation = 0.0f;
	DirectX::XMFLOAT4X4 matrixArr[2];
	if (!CreateVSConstantBuffer(device, vsConstantBuffer, matrixArr, rotation, WIDTH, HEIGHT))
	{
		std::cerr << "Failed to create vertex shader constant buffer!" << std::endl;
		return -1;
	}

	if (!CreatePointLight(device, lightConstantBuffer))
	{
		std::cerr << "Failed to create the pointlight constant buffer!" << std::endl;
		return -1;
	}

	MSG msg = { };

	auto lastTime = std::chrono::high_resolution_clock::now();

	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> deltaTime = currentTime - lastTime;
		lastTime = std::chrono::high_resolution_clock::now();
		rotation += deltaTime.count() * 0.6f;
		if (rotation > 360) rotation = 0;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		ZeroMemory(&mappedResource, sizeof(mappedResource));

		immediateContext->Map(vsConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		DirectX::XMStoreFloat4x4(&matrixArr[0], CreateWorldMatrix(rotation));
		memcpy(mappedResource.pData, matrixArr, sizeof(matrixArr));
		immediateContext->Unmap(vsConstantBuffer, 0);
		
		Render(immediateContext, rtv, dsView, viewport, vShader, pShader, inputLayout, vertexBuffer, vsConstantBuffer, lightConstantBuffer, srv, samplerState);
		swapChain->Present(0, 0);
	}

	samplerState->Release();
	srv->Release();
	lightConstantBuffer->Release();
	vsConstantBuffer->Release();
	vertexBuffer->Release();
	inputLayout->Release();
	pShader->Release();
	vShader->Release();
	dsView->Release();
	dsTexture->Release();
	rtv->Release();
	swapChain->Release();
	immediateContext->Release();
	device->Release();

	return 0;
}
