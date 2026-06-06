#include "Objects.h"

Objects::Objects(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool shouldtessellate, bool isStatic, float angle, bool SRT)
{
	Initialize(device, folderPath, objFile, position, rotation, scale, shouldtessellate, isStatic, angle, SRT);
}

Objects::~Objects()
{
	if (this->mesh) 
	{ 
		delete this->mesh; 
		this->mesh = nullptr; 
	}
}

void Objects::Initialize(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool shouldtessellate, bool isStatic, float angle, bool SRT)
{
	this->shouldTessellate = shouldtessellate;
	this->mesh = new MeshD3D11(device, folderPath, objFile);

	this->center = { { position.x, position.y, position.z, 1 } };
	this->centerBuffer.Initialize(device, sizeof(centerBuffer), &center);

	XMMATRIX world = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX RT;
	if (SRT)
		RT = XMMatrixMultiply(XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z), XMMatrixTranslation(position.x, position.y, position.z));
	else
		RT = XMMatrixMultiply(XMMatrixTranslation(position.x, position.y, position.z), XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z));
	world = XMMatrixMultiply(world, RT);

	this->worldMatrix = world;
	if (!isStatic)
	{
		this->isStatic = false;
		this->updateInf = { position, rotation, scale, angle };

	}

	//XMMATRIX world = XMMatrixScaling(1, 1, 1) * XMMatrixRotationRollPitchYaw(0, 0, 0) * XMMatrixTranslation(1, 1, 10);

	XMFLOAT4X4 world4x4T;
	XMStoreFloat4x4(&world4x4T, XMMatrixTranspose(world));
	this->worldMatrixBuffer.Initialize(device, sizeof(XMFLOAT4X4), &world4x4T);


	this->updateInf.orbitPhase = atan2(this->updateInf.translate.x, this->updateInf.translate.z);
}

void Objects::UpdateObject(ID3D11DeviceContext* context, float deltatime)
{
	if (!isStatic)
	{
		this->updateInf.angle += 0.3f * deltatime;
		this->updateInf.angle = fmod(this->updateInf.angle, XM_2PI);

		this->updateInf.translate.x = 2 * sin(this->updateInf.angle + this->updateInf.orbitPhase);
		this->updateInf.translate.z = 2 * cos(this->updateInf.angle + this->updateInf.orbitPhase);
		this->updateInf.translate.y = 0.1f * sin(this->updateInf.angle * 1.5f) + 0.4f;

 		XMVECTOR scale = XMLoadFloat3(&this->updateInf.scale);
		XMFLOAT3 rotate = this->updateInf.rotate;
		rotate.y += updateInf.angle;
		
		XMMATRIX world =
			XMMatrixScalingFromVector(scale) *
			XMMatrixRotationRollPitchYaw(rotate.x, rotate.y, rotate.z) *
			XMMatrixTranslation(this->updateInf.translate.x, this->updateInf.translate.y, this->updateInf.translate.z);
		XMFLOAT4X4 world4x4;
		XMStoreFloat4x4(&world4x4, XMMatrixTranspose(world));
		this->worldMatrixBuffer.UpdateBuffer(context, &world4x4);

		this->center = { {this->updateInf.translate.x, this->updateInf.translate.y, this->updateInf.translate.z,1} };
		this->centerBuffer.UpdateBuffer(context, &center);
	}
}

void Objects::drawObject(ID3D11DeviceContext* context)
{
	ID3D11Buffer *pMatrix = this->worldMatrixBuffer.GetBuffer();
	context->VSSetConstantBuffers(1, 1, &pMatrix);
	this->mesh->BindMeshBuffers(context);

	size_t nrOfSubMeshes = this->mesh->GetNrOfSubMeshes();

	for (size_t i = 0; i < nrOfSubMeshes; i++)
	{
		this->mesh->PerformSubMeshDrawCall(context, i);
	}
}

ID3D11Buffer* Objects::GetCenterBuffer() { return this->centerBuffer.GetBuffer(); }
