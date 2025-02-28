#include "Renderer.h"
#include "PipelineHelper.h"
#include <iostream>

Renderer::Renderer(Window& window) : window(window), device(nullptr), immediateContext(nullptr), swapChain(nullptr), rtv(nullptr), dsTexture(nullptr),
                                     dsView(nullptr), viewport(), vShader(nullptr), pShader(nullptr), inputLayout(nullptr), vertexBuffer(nullptr), 
                                     vsConstantBuffer(nullptr), psConstantBuffer(nullptr), texture(nullptr), srv(nullptr), samplerState(nullptr) {

    if (!Renderer::Initialize()) {
        std::cerr << "Failed to initialize renderer!" << std::endl;
    }
}

Renderer::~Renderer() {
	if (samplerState) samplerState->Release();
	if (srv) srv->Release();
	if (texture) texture->Release();
	if (psConstantBuffer) psConstantBuffer->Release();
	if (vsConstantBuffer) vsConstantBuffer->Release();
	if (vertexBuffer) vertexBuffer->Release();
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
    if (!SetupRenderTarget()) {
		std::cerr << "Failed to setup render target!" << std::endl;
		return false;
    }
    if (!SetupDepthStencil()) {
		std::cerr << "Failed to setup depth stencil!" << std::endl;
		return false;
    }
    if (!SetupViewport()) {
		std::cerr << "Failed to setup viewport!" << std::endl;
		return false;
    }
	// End of SetupD3D11

    if (!SetupPipeline(device, vertexBuffer, vShader, pShader, inputLayout, texture, srv, samplerState)) {
        std::cerr << "Failed to setup pipeline!" << std::endl;
        return false;
    }

    if (!CreateVSConstantBuffer(device, vsConstantBuffer, matrixArr, rotation, window.GetWidth(), window.GetHeight())) {
        std::cerr << "Failed to create vertex shader constant buffer!" << std::endl;
        return false;
    }

    if (!CreatePointLight(device, psConstantBuffer)) {
        std::cerr << "Failed to create the pointlight constant buffer!" << std::endl;
        return false;
    }
    
    return true;
}

void Renderer::Render() {

    Rotate(); // Rotation math for the Quad

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
    immediateContext->PSSetSamplers(0, 1, &samplerState);
    immediateContext->PSSetConstantBuffers(0, 1, &psConstantBuffer);
    immediateContext->OMSetRenderTargets(1, &rtv, dsView);

    immediateContext->Draw(4, 0);
    swapChain->Present(0, 0);
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

bool Renderer::SetupRenderTarget() {
    // get the address of the back buffer
    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
    {
        std::cerr << "Failed to get back buffer!" << std::endl;
        return false;
    }

    // use the back buffer address to create the render target
    // null as description to base it on the backbuffers values
    HRESULT hr = device->CreateRenderTargetView(backBuffer, NULL, &rtv);
    backBuffer->Release();
    return !(FAILED(hr));
}

bool Renderer::SetupDepthStencil() {

    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = window.GetWidth();
    textureDesc.Height = window.GetHeight();
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &dsTexture)))
    {
        std::cerr << "Failed to create depth stencil texture!" << std::endl;
        return false;
    }

    HRESULT hr = device->CreateDepthStencilView(dsTexture, 0, &dsView);
    return !(FAILED(hr));
}

bool Renderer::SetupViewport() {
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(window.GetWidth());
    viewport.Height = static_cast<float>(window.GetHeight());
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    immediateContext->RSSetViewports(1, &viewport);
    return true;
}

bool Renderer::CreateVSConstantBuffer(ID3D11Device* device, ID3D11Buffer*& vsConstantBuffer, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT) {
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

bool Renderer::CreatePointLight(ID3D11Device* device, ID3D11Buffer*& psConstantBuffer) {

    struct lightStruct
    {
        DirectX::XMFLOAT4 lightPosition = { 0.0f, 0.5f, -5.0f, 0.0f };
        DirectX::XMFLOAT4 lightColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4 cameraPosition = { 0.0f, 0.0f, -3.0f, 1.0f };
        float lightIntensity = 0.01f;
        float shininess = 100.0f;
        char padding[8] = { ' ' };
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

    HRESULT hr = device->CreateBuffer(&lightConstantBufferDesc, &lightData, &psConstantBuffer);
    return !FAILED(hr);
}

void Renderer::Rotate()
{
    time.Update();
    rotation += time.GetDeltaTime() * 0.6f;
    if (rotation > 360) rotation = 0;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(mappedResource));

    immediateContext->Map(vsConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    DirectX::XMStoreFloat4x4(&(matrixArr[0]), CreateWorldMatrix(rotation));
    memcpy(mappedResource.pData, matrixArr, sizeof(matrixArr));
    immediateContext->Unmap(vsConstantBuffer, 0);
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