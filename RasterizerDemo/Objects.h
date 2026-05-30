#pragma once

#include "MeshD3D11.h"
#include "Transform.h"
#include "ConstantBufferD3D11.h"

using namespace DirectX;

class Objects
{
	XMMATRIX worldMatrix;
	ConstantBufferD3D11 worldMatrixBuffer;
	MeshD3D11 *mesh;

public:
	Objects() = default;
	Objects(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT = true);
	~Objects();

	void Initialize(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT = true);

	void UpdateObject(ID3D11DeviceContext* context, float deltatime);
	void drawObject(ID3D11DeviceContext* context);

};
