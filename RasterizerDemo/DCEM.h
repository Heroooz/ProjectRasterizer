#pragma once
#include <d3d11_4.h>
#include <iostream>
#include "CameraD3D11.h"
#include "Objects.h"
//#include "MeshD3D11.h"

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

	MeshD3D11* mesh;
	XMMATRIX worldMatrix;
	ConstantBufferD3D11 worldMatrixBuffer;

	CameraD3D11 cameras[6];

	ID3D11UnorderedAccessView* uav[6];
	//ID3D11RenderTargetView* rtv[6];
	ID3D11ShaderResourceView* srv;
	ID3D11DepthStencilView* dsView;
	D3D11_VIEWPORT viewport;

public:
	DCEM(ID3D11Device* device, XMFLOAT3 initPos);
	~DCEM();

	void Initialize(ID3D11Device* device, XMFLOAT3 initPos);

	void Update(ID3D11DeviceContext* context);
	void Draw(ID3D11DeviceContext* context);


	ID3D11Buffer* GetCameraVP(int nr);
	const ID3D11UnorderedAccessView* GetUAV(int nr);
	const ID3D11RenderTargetView* GetRTV(int nr);
	const ID3D11ShaderResourceView* GetSRV();
	const ID3D11DepthStencilView* GetdsView();
	const D3D11_VIEWPORT GetViewport();
};
