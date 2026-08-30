#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <iostream>

#include "Window.h"
#include "PipelineHelper.h"

#include "DepthBufferD3D11.h"
#include "VertexBufferD3D11.h"
#include "RenderTargetD3D11.h"
#include "ConstantBufferD3D11.h"
#include "ParticlesBuffer.h"

#include "CameraD3D11.h"
#include "GBuffer.h"
#include "Scene.h"
#include "SimpleVertex.h"
#include "Light.h"


class Renderer {
public:
	Renderer(Window& window, Scene& scene);
	~Renderer() = default;

	bool Initialize(Scene& scene);
	void Render(Scene& scene, bool tesselation, bool shadow, bool particles);

	ID3D11Device* GetDevice();
	CameraD3D11& GetCamera();

	void UpdateParticles(Scene& scene);

	inline void PrintCoordinates() { std::cout << "X: " << camera.GetPosition().x << " Y: " << camera.GetPosition().y << " Z: " << camera.GetPosition().z << std::endl; };

private:
	Window& window;
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> immediateContext;
	ComPtr<IDXGISwapChain> swapChain;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11DepthStencilView> dsView;
	D3D11_VIEWPORT viewport;

	std::unique_ptr<ShaderD3D11> vsShader;
	std::unique_ptr<ShaderD3D11> psShader[2]; 	// DeferredPS[0], DCEMPS[1]
	std::unique_ptr<ShaderD3D11> dcemShader;
	std::unique_ptr<ShaderD3D11> csShader;
	std::unique_ptr<ShaderD3D11> hullShader;
	std::unique_ptr<ShaderD3D11> domainShader;
	std::unique_ptr<ShaderD3D11> particleShaders[4]; 	// VS[0], GS[1], PS[2], CS[3]

	std::unique_ptr<InputLayoutD3D11> inputLayout;
	ComPtr<ID3D11Texture2D> texture;
	ComPtr<ID3D11Buffer> vsConstantBuffer;
	ComPtr<ID3D11Buffer> psConstantBuffer;
	ComPtr<ID3D11ShaderResourceView> srv;
	std::unique_ptr<SamplerD3D11> samplerState;
	std::unique_ptr<SamplerD3D11> shadowSampler;
	std::unique_ptr<SamplerD3D11> DCEMSampler;

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

	CameraD3D11 camera;

	ConstantBufferD3D11 worldMatrixBuffer;
	ComPtr<ID3D11Buffer> pWorldMatrix;

	ConstantBufferD3D11 cameraBuffer;
	ComPtr<ID3D11Buffer> pCamera;

	ComPtr<ID3D11Buffer> lightPS;


	struct RenderingMode {
		int mode = 0;
		float _padding[3]{};
	} renderingMode;
	ConstantBufferD3D11 renderingModeBuffer;

	
	void ClearBuffers();
	void ShadowPass(Scene& scene, bool tessellate);
	void GeometryPass(Scene& scene, bool tessellate);
	void LightPass(Scene& scene, bool shadow);

	void Tesselate(bool tesselation);
	void DrawParticles(Scene& scene);
	void DrawObjects(Scene& scene, bool tessellation);

	// Adding the the scene
	void CreateLights(ComPtr<ID3D11Device> device, Scene& scene);
	void LoadObjects(Scene& scene);
	void InitializeParticles(Scene& scene);


	// Helpful set-up functions
	bool SetupDeviceAndSwapChain();
	void SetupRenderTarget();
	void SetupDepthStencil();
	void SetupViewport();
	bool CreateUnorderedAccessView();
};
