#include "Scene.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::AddObject(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT)
{
	Objects* obj = new Objects(device, folderPath, objFile, position, rotation, scale, SRT);
	objects.push_back(obj);
}

void Scene::AddLight()
{
}

void Scene::DrawScene(ID3D11DeviceContext* context)
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
