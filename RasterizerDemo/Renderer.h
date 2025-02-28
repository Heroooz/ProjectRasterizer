#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "Window.h"
#include "Time.h"

class Renderer {
public:
    Renderer(Window& window);
    ~Renderer();

    bool Initialize();
    void Render();

private:
    Window& window;
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
	ID3D11Buffer* psConstantBuffer; 
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* samplerState;

	Time time;
	float rotation = 0.0f;
	DirectX::XMFLOAT4X4 matrixArr[2] = {};

    bool SetupDeviceAndSwapChain();
    bool SetupRenderTarget();
    bool SetupDepthStencil();
    bool SetupViewport();

	bool CreateVSConstantBuffer(ID3D11Device* device, ID3D11Buffer*& vsConstantBuffer, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT);
	bool CreatePointLight(ID3D11Device* device, ID3D11Buffer*& lightConstantBuffer);

	void Rotate();
};

DirectX::XMMATRIX CreateWorldMatrix(float angle);
DirectX::XMMATRIX CreateViewMatrix();
DirectX::XMMATRIX CreateProjectionMatrix(const float fovAngle, const float aspectRatio, const float nearZ, const float farZ);
