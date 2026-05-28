#pragma once

#include "MeshD3D11.h"
#include "Transform.h"

using namespace DirectX;

class Objects
{
	DirectX::XMFLOAT4X4 worldMatrix;
	MeshD3D11 mesh;


	/*Transform worldMatrix =
	{
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 1, 1, 1}
	};*/

public:
	Objects();
	Objects(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT = true);
	~Objects();

	void Initialize(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT = true);

};
