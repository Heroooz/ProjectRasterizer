#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>

#include "Window.h"
#include "Time.h"
#include "CameraD3D11.h"
#include "DepthBufferD3D11.h"
#include "VertexBufferD3D11.h"
#include "RenderTargetD3D11.h"
#include "ConstantBufferD3D11.h"
#include "ShaderResourceTextureD3D11.h"
#include "GBuffer.h"
#include "Scene.h"

#include "PipelineHelper.h"
#include "SimpleVertex.h"
#include "Light.h"
#include "Transform.h"


class Renderer {
public:
    Renderer(Window& window);
    ~Renderer();

    bool Initialize();
    void Render();

	CameraD3D11& GetCamera();
private:

	const unsigned int NR_OF_GBUFFERS = 2;

	double PI = 3.14159265358979323846;

    Window& window;
	ID3D11Device* device;
	ID3D11DeviceContext* immediateContext;
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* rtv;
	ID3D11Texture2D* dsTexture;
	ID3D11DepthStencilView* dsView;
	D3D11_VIEWPORT viewport;

	//ID3D11RenderTargetView* rtvArr[2];
	//ID3D11UnorderedAccessView* uav;

	ShaderD3D11* vsShader;
	ShaderD3D11* psShader;
	ShaderD3D11* csShader;
	ID3D11VertexShader* vShader;
	ID3D11PixelShader* pShader;
	//ID3D11InputLayout* inputLayout;
	InputLayoutD3D11* inputLayout;
	ID3D11Texture2D* texture;
	ID3D11Buffer* vsConstantBuffer;
	ID3D11Buffer* psConstantBuffer;
	ID3D11ShaderResourceView* srv;
	SamplerD3D11* samplerState;
	//ID3D11SamplerState* samplerState;
	ID3D11Buffer* vertexBuffer;


	// FOr the quads
	VertexBufferD3D11 vertexBuffers[10];
	DirectX::XMMATRIX worldMatrices[10];
	ConstantBufferD3D11 worldMatriceBuffers[10];

	// G-Buffers and null-buffers
	GBuffer positionBuffer;
	GBuffer normalBuffer;
	GBuffer diffuseBuffer;
	ID3D11RenderTargetView* rtvArr[3];
	ID3D11RenderTargetView* rtvNULL[3] = { nullptr, nullptr, nullptr };;
	ID3D11ShaderResourceView* srvArr[3];
	ID3D11ShaderResourceView* srvNULL[3] = { nullptr, nullptr, nullptr };;
	ID3D11UnorderedAccessView* uav;
	ID3D11UnorderedAccessView* uavNULL = nullptr;


	RenderTargetD3D11 renderTargetD3D11;
	DepthBufferD3D11 depthBufferD3D11;
	ConstantBufferD3D11 vsConstantBufferD3D11;
	ConstantBufferD3D11 psConstantBufferD3D11;
	VertexBufferD3D11 vertexBufferD3D11;

	CameraD3D11 camera;
	Scene *scene = new Scene();

	ConstantBufferD3D11 worldMatrixBuffer;
	ID3D11Buffer* pWorldMatrix;

	ConstantBufferD3D11 cameraBuffer;
	ID3D11Buffer* pCamera;

	ID3D11Buffer* lightPS;


	Time time;
	float rotation = 0.0f;
	DirectX::XMFLOAT4X4 matrixArr[2] = {}; // World and ViewProjection matrices


	struct CameraBuffer
	{
		XMFLOAT4X4 viewProjMatrix;
		XMFLOAT3 cameraPosition;
		float padding;
	};

	void ClearBuffers();
	void GeometryPass();
	void LightPass();


    bool SetupDeviceAndSwapChain();
    void SetupRenderTarget();
    void SetupDepthStencil();
	void SetupViewport();
	bool CreateUnorderedAccessView();

	void CreateVertexBuffer(ID3D11Device* device, VertexBufferD3D11& vertexBufferD3D11, int nrOfVertices, void* vertexData);
	void CreateVSConstantBuffer(ID3D11Device* device, ConstantBufferD3D11& vsConstantBuffer, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT);
	void CreatePointLight(ID3D11Device* device, ConstantBufferD3D11& lightConstantBuffer);

	void LoadObjects();
};
	DirectX::XMMATRIX CreateWorldMatrix(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);

