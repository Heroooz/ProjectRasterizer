#pragma once

#include "MeshD3D11.h"
#include "Transform.h"
#include "ConstantBufferD3D11.h"

using namespace DirectX;

class Objects
{
	ConstantBufferD3D11 worldMatrix;
	MeshD3D11 *mesh;

public:
	Objects();
	Objects(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT = true);
	~Objects();

	void Initialize(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT = true);

	void UpdateObject(ID3D11DeviceContext* context);
	void drawObject(ID3D11DeviceContext* context);

};
