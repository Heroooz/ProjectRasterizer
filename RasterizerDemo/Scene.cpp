#include "Scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
	for (auto& object : objects)
	{
		if (object != nullptr)
		{
			object->~Objects();
		}
	}
	this->objects.clear();
}

void Scene::AddObject(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool isStatic, float angle, bool SRT)
{
	Objects* obj = new Objects(device, folderPath, objFile, position, rotation, scale, isStatic, angle, SRT);
	objects.push_back(obj);
}

void Scene::AddLight(ID3D11Device* device, LightData data)
{	
	Light light;
	light.Initialize(device, data);
	if (data.perLightInfo.isDir) dirLights.Initialize(device, data);
	else spotLights.Initialize(device, data);
}

void Scene::InitializeLight(ID3D11Device* device)
{
	this->spotLights.InitializeBuffers(device);
	this->dirLights.InitializeBuffers(device);

	NrOfLights nrof;
	nrof.nrofSpotLights = this->spotLights.GetNrOfLights();
	nrof.nrofDirLights = this->dirLights.GetNrOfLights();

	this->nrofLights.Initialize(device, sizeof(NrOfLights), &nrof);
}

//void Scene::SetNrOfLights(ID3D11Device* device)
//{
//
//}


void Scene::UpdateObjects(ID3D11DeviceContext* context, float deltatime)
{
	for (auto& object : objects)
		object->UpdateObject(context, deltatime);
}

void Scene::DrawScene(ID3D11DeviceContext* context)
{
}

void Scene::DrawObjects(ID3D11DeviceContext* context)
{
	for (auto& obj : objects)
	{
		obj->drawObject(context);
	}
}


void Scene::RemoveObjectFromScene(int index)
{
}

ID3D11Buffer* Scene::GetnrofLightBuffer()
{
	return this->nrofLights.GetBuffer();
}

ID3D11ShaderResourceView* Scene::GetLightBufferSRV(bool isDir)
{
	if (isDir) return this->dirLights.GetLightBufferSRV();
	else return this->spotLights.GetLightBufferSRV();
}

ID3D11ShaderResourceView* Scene::GetShadowMapSRV(bool isDir)
{
	if (isDir) return this->dirLights.GetShadowMapsSRV();
	else return this->spotLights.GetShadowMapsSRV();
}



int Scene::GetNrOfObjects() { return this->objects.size(); }

int Scene::GetNrOfSpotLights() { return this->spotLights.GetNrOfLights(); }

int Scene::GetNrOfDirLight() { return this->dirLights.GetNrOfLights(); }
