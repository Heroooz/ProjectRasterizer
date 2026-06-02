#pragma once

#include "MeshD3D11.h"
#include "Transform.h"
#include "ConstantBufferD3D11.h"

using namespace DirectX;

struct UpdateInfo
{
	XMFLOAT3 translate = { 0,0,0 };
	XMFLOAT3 rotate = { 0,0,0 };
	XMFLOAT3 scale{ 1,1,1 };
	float angle = 0.0f;
};

class Objects
{
	XMMATRIX worldMatrix;
	ConstantBufferD3D11 worldMatrixBuffer;
	MeshD3D11 *mesh;
	bool isStatic = true;
	UpdateInfo updateInf;

public:
	Objects() = default;
	Objects(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool isStatic, float angle, bool SRT);
	~Objects();

	void Initialize(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool isStatic, float angle, bool SRT);

	void UpdateObject(ID3D11DeviceContext* context, float deltatime);
	void drawObject(ID3D11DeviceContext* context);

};
