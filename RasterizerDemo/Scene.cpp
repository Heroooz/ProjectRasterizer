#include "Scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
	for (auto& object : objects)
	{
		object->~Objects();
	}
}

void Scene::AddObject(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT)
{
	Objects* obj = new Objects(device, folderPath, objFile, position, rotation, scale, SRT);
	objects.push_back(obj);
}

void Scene::AddLight(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity)
{
	Light* light = new Light(device, color, position, intensity);
	lights.push_back(light);
}


void Scene::DrawScene(ID3D11DeviceContext* context)
{
	for (auto& light : lights)
	{ 
		ID3D11Buffer* pLight = light->GetBuffer()->GetBuffer();
		light->UpdateBuffer(context);
		context->VSSetConstantBuffers(1, 1, &pLight);
	}
	for (auto& obj : objects)
	{
		obj->drawObject(context);
	}
}

void Scene::RemoveObjectFromScene(int index)
{
}

int Scene::getNrOfObjects() { return this->objects.size(); }

int Scene::getNrOfLight() { return this->lights.size(); }
