#pragma once
#include <d3d11_4.h>
#include <iostream>
#include "CameraD3D11.h"
#include "ShaderD3D11.h"
#include "Objects.h"
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

using namespace DirectX;

class DCEM
{
	enum D3D11_TEXTURECUBE_FACE {
		POSITIVE_X = 0,
		NEGATIVE_X = 1,
		POSITIVE_Y = 2,
		NEGATIVE_Y = 3,
		POSITIVE_Z = 4,
		NEGATIVE_Z = 5
	};

	std::unique_ptr<MeshD3D11> mesh;
	XMMATRIX worldMatrix;
	ConstantBufferD3D11 worldMatrixBuffer;

	CameraD3D11 cameras[6];

	ComPtr<ID3D11RenderTargetView> rtv[6];
	ComPtr<ID3D11DepthStencilView> dsView;
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11Texture2D> texture;
	D3D11_VIEWPORT viewport;


	ShaderD3D11* DCEMPS;
	ShaderD3D11* normalPS;
	ShaderD3D11* cubeMapCapturePS;

public:
	DCEM(ID3D11Device* device, XMFLOAT3 initPos, UINT width, UINT height, std::unique_ptr<ShaderD3D11>& DCEMPS, std::unique_ptr<ShaderD3D11>& normalPS, std::unique_ptr<ShaderD3D11>& DCEMCapturePS, std::string objName);
	~DCEM() = default;

	void Initialize(ID3D11Device* device, XMFLOAT3 initPos, UINT width, UINT height);

	void Update(ID3D11DeviceContext* context);
	void GenerateCubemap(ID3D11DeviceContext* context, const std::vector<Objects*>& sceneObjects);
	void Draw(ID3D11DeviceContext* context);


	ID3D11Buffer* GetCameraVP(int nr);
	const ID3D11RenderTargetView* GetRTV(int nr);
	const ID3D11ShaderResourceView* GetSRV();
	const ID3D11DepthStencilView* GetdsView();
	const D3D11_VIEWPORT GetViewport();
};
