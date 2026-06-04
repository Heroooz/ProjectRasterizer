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
			delete object;
			object = nullptr;
		}
	}
	if (pCenter)
	{
		pCenter->Release();
		pCenter = nullptr;
	}
}

void Scene::AddDCEM(ID3D11Device* device, XMFLOAT3 position, UINT width, UINT height, std::unique_ptr<ShaderD3D11>& DCEMPS, std::unique_ptr<ShaderD3D11>& normalPS, bool isSkyBox)
{
	dcems.push_back(new DCEM(device, position, width, height, DCEMPS, normalPS));
}

void Scene::AddObject(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool isStatic, float angle, bool SRT)
{
	objects.push_back(new Objects(device, folderPath, objFile, position, rotation, scale, isStatic, angle, SRT));
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
	for (auto dcem : dcems)
		dcem->Update(context);
	for (auto& object : objects)
		object->UpdateObject(context, deltatime);
}

void Scene::DrawScene(ID3D11DeviceContext* context)
{
}

void Scene::DrawObjects(ID3D11DeviceContext* immediatecontext, bool tesselate)
{
	//int nrof = this->GetNrOfObjects();
	for (auto& obj : objects)
	{
		if (tesselate)
		{
			pCenter = obj->GetCenterBuffer();
			immediatecontext->HSSetConstantBuffers(1, 1, &pCenter);
		}
		obj->drawObject(immediatecontext);
	}
}

void Scene::DrawDCEM(ID3D11DeviceContext* context)
{
	for (auto& dcem : dcems)
	{
		dcem->GenerateCubemap(context, objects);
		dcem->Draw(context);
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



int Scene::GetNrOfObjects() { return (int)this->objects.size(); }

int Scene::GetNrOfSpotLights() { return this->spotLights.GetNrOfLights(); }

int Scene::GetNrOfDirLight() { return this->dirLights.GetNrOfLights(); }
