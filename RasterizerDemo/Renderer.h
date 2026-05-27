#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Window.h"
#include "Time.h"
#include "CameraD3D11.h"
#include "DepthBufferD3D11.h"
#include "VertexBufferD3D11.h"
#include "RenderTargetD3D11.h"
#include "ConstantBufferD3D11.h"
#include "MeshD3D11.h"


class Renderer {
public:
    Renderer(Window& window);
    ~Renderer();

    bool Initialize();
    void Render();

	void loadObjects();

	CameraD3D11& GetCamera();
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
	ID3D11Texture2D* texture;
	ID3D11Buffer* vsConstantBuffer;
	ID3D11Buffer* psConstantBuffer;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* samplerState;
	ID3D11Buffer* vertexBuffer;

	VertexBufferD3D11 vertexBuffers[10];
	DirectX::XMMATRIX worldMatrices[10];
	ConstantBufferD3D11 worldMatriceBuffers[10];

	std::vector<MeshD3D11*> objs;
	std::vector<DirectX::XMMATRIX> objsWorldMatrices;
	std::vector<ConstantBufferD3D11*> objsWorldMatrixBuffers;


	RenderTargetD3D11 renderTargetD3D11;
	DepthBufferD3D11 depthBufferD3D11;
	ConstantBufferD3D11 vsConstantBufferD3D11;
	ConstantBufferD3D11 psConstantBufferD3D11;
	VertexBufferD3D11 vertexBufferD3D11;

	CameraD3D11 camera;

	ConstantBufferD3D11 worldMatrixBuffer;
	ID3D11Buffer* pWorldMatrix;

	ConstantBufferD3D11 cameraBuffer;
	ID3D11Buffer* pCamera;

	Time time;
	float rotation = 0.0f;
	DirectX::XMFLOAT4X4 matrixArr[2] = {}; // World and ViewProjection matrices

    bool SetupDeviceAndSwapChain();
    void SetupRenderTarget();
    void SetupDepthStencil();
	void SetupViewport();

	void CreateVertexBuffer(ID3D11Device* device, VertexBufferD3D11& vertexBufferD3D11, int nrOfVertices, void* vertexData);
	void CreateVSConstantBuffer(ID3D11Device* device, ConstantBufferD3D11& vsConstantBuffer, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT);
	void CreatePointLight(ID3D11Device* device, ConstantBufferD3D11& lightConstantBuffer);
};

DirectX::XMMATRIX CreateWorldMatrix(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);
//DirectX::XMMATRIX CreateViewMatrix();
//DirectX::XMMATRIX CreateProjectionMatrix(const float fovAngle, const float aspectRatio, const float nearZ, const float farZ);
