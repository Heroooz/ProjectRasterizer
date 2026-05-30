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


void Scene::UpdateObjects(ID3D11DeviceContext* context, float deltatime)
{
}

void Scene::DrawScene(ID3D11DeviceContext* context)
{
	for (auto& light : lights)
	{
		light->UpdateBuffer(context);
		ID3D11Buffer* pLight = light->GetBuffer()->GetBuffer();
		context->PSSetConstantBuffers(1, 1, &pLight);
		for (auto& obj : objects)
		{
			obj->drawObject(context);
		}
	}
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

int Scene::getNrOfObjects() { return this->objects.size(); }

int Scene::getNrOfLight() { return this->lights.size(); }
