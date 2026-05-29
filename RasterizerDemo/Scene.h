#pragma once

#include "Transform.h"
#include "Objects.h"
#include "Light.h"


class Scene
{
	std::vector<Objects*> objects;
	std::vector<Light*> lights;


public:
	Scene();
	~Scene();

	void Initialize();

	void AddObject(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool SRT = true);
	void AddLight(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity);

	void DrawScene(ID3D11DeviceContext* context);

	void RemoveObjectFromScene(int index);

	int getNrOfObjects();
	int getNrOfLight();
};
