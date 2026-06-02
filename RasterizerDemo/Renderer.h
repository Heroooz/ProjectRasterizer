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
//#include "ShaderResourceTextureD3D11.h"
#include "GBuffer.h"
#include "Scene.h"

#include "PipelineHelper.h"
#include "SimpleVertex.h"
#include "Light.h"
#include "Transform.h"


class Renderer {
public:
	Renderer(Window& window);
	~Renderer() = default;

	bool Initialize();
	void Render();

	ID3D11DeviceContext* GetContext();
	CameraD3D11& GetCamera();
	Scene& GetScene();
	float GetDeltatime();
private:

	const unsigned int NR_OF_GBUFFERS = 2;

	double PI = 3.14159265358979323846;

	Window& window;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> immediateContext;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> rtv;
	//ComPtr<ID3D11Texture2D> dsTexture;
	ComPtr<ID3D11DepthStencilView> dsView;
	D3D11_VIEWPORT viewport;

	std::unique_ptr<ShaderD3D11> vsShader;
	std::unique_ptr<ShaderD3D11> psShader[2];
	std::unique_ptr<ShaderD3D11> csShader;
	std::unique_ptr<ShaderD3D11> hullShader;
	std::unique_ptr<ShaderD3D11> domainShader;

	ID3D11HullShader* pHullShader;
	ID3D11DomainShader* pDomainShader;

	std::unique_ptr<InputLayoutD3D11> inputLayout;
	ComPtr<ID3D11Texture2D> texture;
	ComPtr<ID3D11Buffer> vsConstantBuffer;
	ComPtr<ID3D11Buffer> psConstantBuffer;
	ComPtr<ID3D11ShaderResourceView> srv;
	std::unique_ptr<SamplerD3D11> samplerState;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> pvertexBuffer;

	// For the quads
	VertexBufferD3D11 vertexBuffer;
	DirectX::XMMATRIX worldMatrices[2];
	ConstantBufferD3D11 worldMatriceBuffers[2];

	// G-Buffers and null-buffers
	GBuffer positionBuffer;
	GBuffer normalBuffer;
	GBuffer diffuseBuffer;
	ComPtr<ID3D11RenderTargetView> rtvArr[3];
	ComPtr<ID3D11RenderTargetView> rtvNULL[3] = { nullptr, nullptr, nullptr };;
	ComPtr<ID3D11ShaderResourceView> srvArr[3];
	ComPtr<ID3D11ShaderResourceView> srvNULL[3] = { nullptr, nullptr, nullptr };;
	ComPtr<ID3D11UnorderedAccessView> uav;
	ComPtr<ID3D11UnorderedAccessView> uavNULL = nullptr;


	RenderTargetD3D11 renderTargetD3D11;
	DepthBufferD3D11 depthBufferD3D11;
	ConstantBufferD3D11 vsConstantBufferD3D11;
	ConstantBufferD3D11 psConstantBufferD3D11;
	//VertexBufferD3D11 vertexBufferD3D11;

	CameraD3D11 camera;
	std::unique_ptr<Scene> scene = std::make_unique<Scene>();

	ConstantBufferD3D11 worldMatrixBuffer;
	ComPtr<ID3D11Buffer> pWorldMatrix;

	ConstantBufferD3D11 cameraBuffer;
	ComPtr<ID3D11Buffer> pCamera;

	ComPtr<ID3D11Buffer> lightPS;

	bool ShouldTesselate = false;


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

	void CreateVertexBuffer(ComPtr<ID3D11Device> device, VertexBufferD3D11& vertexBufferD3D11, int nrOfVertices, void* vertexData);
	void CreateVSConstantBuffer(ComPtr<ID3D11Device> device, ConstantBufferD3D11& vsConstantBuffer, DirectX::XMFLOAT4X4 matrixArr[], float rotation, UINT WIDTH, UINT HEIGHT);

	void CreateLights(ComPtr<ID3D11Device> device);
	void LoadObjects();
};
	DirectX::XMMATRIX CreateWorldMatrix(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);

