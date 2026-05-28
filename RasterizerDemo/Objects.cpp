#include "Objects.h"

Objects::Objects()
{
	
}

Objects::Objects(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT)
{
	Initialize(device, folderPath, objFile, position, rotation, scale, SRT);
}

Objects::~Objects()
{
	this->mesh->~MeshD3D11();
	
}

void Objects::Initialize(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT)
{
	this->mesh = new MeshD3D11(device, folderPath, objFile);

	XMMATRIX world = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX RT;
	if (SRT)
		RT = XMMatrixMultiply(XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z), XMMatrixTranslation(position.x, position.y, position.z));
	else
		RT = XMMatrixMultiply(XMMatrixTranslation(position.x, position.y, position.z), XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z));
	world = XMMatrixMultiply(world, RT);


	//XMMATRIX world = XMMatrixScaling(1, 1, 1) * XMMatrixRotationRollPitchYaw(0, 0, 0) * XMMatrixTranslation(1, 1, 10);

	XMFLOAT4X4 world4x4T;
	XMStoreFloat4x4(&world4x4T, XMMatrixTranspose(world));
	this->worldMatrix.Initialize(device, sizeof(XMFLOAT4X4), &world4x4T);
}

void Objects::UpdateObject()
{
}

void Objects::drawObject(ID3D11DeviceContext* context)
{
	ID3D11Buffer *pMatrix = this->worldMatrix.GetBuffer();
	context->VSSetConstantBuffers(1, 1, &pMatrix);
	this->mesh->BindMeshBuffers(context);
	int nrOfSubMeshes = this->mesh->GetNrOfSubMeshes();

	for (size_t i = 0; i < nrOfSubMeshes; i++)
	{
		this->mesh->PerformSubMeshDrawCall(context, i);
	}
}
