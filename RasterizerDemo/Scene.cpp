#include "Scene.h"

void Scene::AddObject()
{
}

void Scene::AddLight()
{
}

void Scene::DrawScene(ID3D11DeviceContext* context)
{
	for (auto& obj : objects)
	{
		obj->BindMeshBuffers(context);

		int nrOfSubMeshes = obj->GetNrOfSubMeshes();
		for (int i = 0; i < nrOfSubMeshes; i++)
		{
			obj->PerformSubMeshDrawCall(context, i);
		}
	}
}

void Scene::RemoveObjectFromScene()
{
}

int Scene::getNrOfObjects() { return this->objects.size(); }

int Scene::getNrOfLight() { return this->lights.size(); }
