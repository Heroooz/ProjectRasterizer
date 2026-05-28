#pragma once

#include "Transform.h"
#include "MeshD3D11.h"
#include "Light.h"


class Scene
{
	std::vector<MeshD3D11*> objects;
	std::vector<lightStruct*> lights;


public:
	Scene();
	~Scene();

	void Initialize();

	void AddObject();
	void AddLight();

	void DrawScene(ID3D11DeviceContext* context);

	void RemoveObjectFromScene();

	int getNrOfObjects();
	int getNrOfLight();
};

Scene::Scene()
{
}

Scene::~Scene()
{
}