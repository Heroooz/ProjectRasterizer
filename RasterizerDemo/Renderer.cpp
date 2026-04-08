#include "Renderer.h"
#include "PipelineHelper.h"
#include <iostream>
#include "SimpleVertex.h"
#include "Light.h"

Renderer::Renderer(Window& window) : window(window), device(nullptr), immediateContext(nullptr), swapChain(nullptr), rtv(nullptr), dsTexture(nullptr),
                                     dsView(nullptr), viewport(), vShader(nullptr), pShader(nullptr), inputLayout(nullptr), vertexBuffer(nullptr), 
                                     vsConstantBuffer(nullptr), psConstantBuffer(nullptr), texture(nullptr), srv(nullptr), samplerState(nullptr), 
                                     renderTargetD3D11(), depthBufferD3D11(), vsConstantBufferD3D11(), psConstantBufferD3D11(),
                                     vertexBufferD3D11(), camera(), rotation(0.0f) {

    if (!Renderer::Initialize()) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
    }
}

Renderer::~Renderer() {
	if (samplerState) samplerState->Release();
	if (srv) srv->Release();
	if (texture) texture->Release();
	if (vertexBuffer) vertexBuffer->Release();
	if (psConstantBuffer) psConstantBuffer->Release();
	if (vsConstantBuffer) vsConstantBuffer->Release();
	if (inputLayout) inputLayout->Release();
	if (pShader) pShader->Release();
	if (vShader) vShader->Release();
	if (dsView) dsView->Release();
	if (dsTexture) dsTexture->Release();
	if (rtv) rtv->Release();
	if (swapChain) swapChain->Release();
	if (immediateContext) immediateContext->Release();
	if (device) device->Release();
}

bool Renderer::Initialize() {

    // SetupD3D11
    if (!SetupDeviceAndSwapChain()) {
		std::cerr << "Failed to setup device and swap chain!" << std::endl;
		return false;
    }
    SetupRenderTarget();
	SetupDepthStencil();
	SetupViewport();
	// End of SetupD3D11

    SimpleVertex triangle[] =
    {
        { {-0.5f, 0.5f, 0.0f}, {0, 0, -1}, {0, 0} },
        { {0.5f, 0.5f, 0.0f}, {0, 0, -1}, {1, 0} },
        { {-0.5, -0.5f, 0.0f}, {0, 0, -1}, {0, 1} },

        { {0.5f, -0.5f, 0.0f}, {0, 0, -1}, {1, 1} }
    };
    CreateVertexBuffer(device, vertexBufferD3D11, 4, triangle);

	// Setup Pipeline, shaders, input layout, texture, sampler state
    if (!SetupPipeline(device, vShader, pShader, inputLayout, texture, srv, samplerState)) {
        std::cerr << "Failed to setup pipeline!" << std::endl;
        return false;
    }

	// Setup constant buffers (for vertex shader and pixel shader)
    CreateVSConstantBuffer(device, vsConstantBufferD3D11, matrixArr, rotation, window.GetWidth(), window.GetHeight());
    CreatePointLight(device, psConstantBufferD3D11);

	// Setup camera (projection info and initialize)
    ProjectionInfo projInfo;
	projInfo.fovAngleY = DirectX::XMConvertToRadians(90.0f);
	projInfo.aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
	projInfo.nearZ = 0.1f;
	projInfo.farZ = 100.0f;
    camera.Initialize(device, projInfo, DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f));

    return true;
}

void Renderer::Render() {

	time.Update(); // Update frame timing

    camera.UpdateInternalConstantBuffer(immediateContext);

    DirectX::XMMATRIX worldMatrix = CreateWorldMatrix(rotation);
    DirectX::XMMATRIX worldViewProjectionMatrix = DirectX::XMMatrixMultiplyTranspose(CreateWorldMatrix(0.0f), camera.GetViewProjectionMatrix());
    
    DirectX::XMStoreFloat4x4(&matrixArr[0], worldMatrix);
    DirectX::XMStoreFloat4x4(&matrixArr[1], worldViewProjectionMatrix);
    
	vsConstantBufferD3D11.UpdateBuffer(immediateContext, &matrixArr);

    float clearColour[4] = { 0, 0, 0, 0 };
    immediateContext->ClearRenderTargetView(rtv, clearColour);
    immediateContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;

	// Bind and set pipeline states, then draw
    immediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    immediateContext->IASetInputLayout(inputLayout);
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    immediateContext->VSSetShader(vShader, nullptr, 0);
    immediateContext->VSSetConstantBuffers(0, 1, &vsConstantBuffer);
    immediateContext->RSSetViewports(1, &viewport);
    immediateContext->PSSetShader(pShader, nullptr, 0);
    immediateContext->PSSetShaderResources(0, 1, &srv);
    immediateContext->PSSetSamplers(0, 1, &samplerState);
    immediateContext->PSSetConstantBuffers(0, 1, &psConstantBuffer);
    immediateContext->OMSetRenderTargets(1, &rtv, dsView);

    immediateContext->Draw(4, 0);
    swapChain->Present(0, 0);
}

CameraD3D11& Renderer::GetCamera()
{
    return camera;
}

bool Renderer::SetupDeviceAndSwapChain() {
    UINT flags = 0;

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    swapChainDesc.BufferDesc.Width = window.GetWidth();
    swapChainDesc.BufferDesc.Height = window.GetHeight();
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Default
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = window.GetHWND();
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, featureLevels, 1, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, nullptr, &immediateContext);

    return !(FAILED(hr));
}

void Renderer::SetupRenderTarget() {
	renderTargetD3D11.Initialize(device, swapChain, window.GetWidth(), window.GetHeight());
	rtv = renderTargetD3D11.GetRTV();
}

void Renderer::SetupDepthStencil() {
	depthBufferD3D11.Initialize(device, window.GetWidth(), window.GetHeight());
    dsView = depthBufferD3D11.GetDSV(0);
}

void Renderer::SetupViewport() {
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(window.GetWidth());
    viewport.Height = static_cast<float>(window.GetHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    immediateContext->RSSetViewports(1, &viewport);
}

void Renderer::CreateVertexBuffer(ID3D11Device* device, VertexBufferD3D11& vertexBufferD3D11, int nrOfVertices, void* vertexData) {
	vertexBufferD3D11.Initialize(device, sizeof(SimpleVertex), nrOfVertices, vertexData);
	vertexBuffer = vertexBufferD3D11.GetBuffer();
}

void Renderer::CreateVSConstantBuffer(ID3D11Device* device, ConstantBufferD3D11& vsConstantBufferD3D11, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT) {
    DirectX::XMMATRIX worldMatrix = CreateWorldMatrix(rotation);
    DirectX::XMFLOAT4X4 worldMatrixFloat4x4;
    DirectX::XMStoreFloat4x4(&worldMatrixFloat4x4, worldMatrix);

    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixMultiplyTranspose(CreateViewMatrix(), CreateProjectionMatrix(90.0f, static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f));
    DirectX::XMFLOAT4X4 viewMatrixFloat4x4;
    DirectX::XMStoreFloat4x4(&viewMatrixFloat4x4, viewMatrix);

    matrixArr[0] = worldMatrixFloat4x4;
    matrixArr[1] = viewMatrixFloat4x4;

	vsConstantBufferD3D11.Initialize(device, sizeof(DirectX::XMFLOAT4X4) * 2, matrixArr);
    vsConstantBuffer = vsConstantBufferD3D11.GetBuffer();
}

void Renderer::CreatePointLight(ID3D11Device* device, ConstantBufferD3D11& psConstantBufferD3D11) {



    lightStruct light;
	psConstantBufferD3D11.Initialize(device, sizeof(lightStruct), &light);
    psConstantBuffer = psConstantBufferD3D11.GetBuffer();
}

DirectX::XMMATRIX CreateWorldMatrix(float angle) {
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