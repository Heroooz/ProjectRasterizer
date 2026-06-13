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
	for (auto& dcem : dcems)
	{
		if (dcem)
		{
			delete dcem;
			dcem = nullptr;
		}
	}
	if (particles)
	{
		delete particles;
		particles = nullptr;
	}
	if (pCenter)
	{
		pCenter = nullptr;
	}
}

void Scene::AddDCEM(ID3D11Device* device, XMFLOAT3 position, UINT width, UINT height, std::unique_ptr<ShaderD3D11>& DCEMPS, std::unique_ptr<ShaderD3D11>& normalPS, std::unique_ptr<ShaderD3D11>& DCEMCapturePS, std::string objName, bool isSkyBox)
{
	dcems.push_back(new DCEM(device, position, width, height, DCEMPS, normalPS, DCEMCapturePS, objName));
}

void Scene::AddObject(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool shouldtessellate, bool isStatic, float angle, bool SRT)
{
	Objects* obj = new Objects(device, folderPath, objFile, position, rotation, scale, shouldtessellate, isStatic, angle, SRT);
	objects.push_back(obj);
	this->quadtree.AddElement(objects.back(), obj->GetBoundingBox());
}

void Scene::AddLight(ID3D11Device* device, LightData data)
{	
	if (data.perLightInfo.isDir) this->dirLights.Initialize(device, data);
	else this->spotLights.Initialize(device, data);
}

void Scene::AddParticles(ID3D11Device* device, UINT sizeOfElement, UINT nrOfElementsInBuffer, void* bufferData, bool dynamic, bool hasSRV, bool hasUAV)
{
	this->particles = new Particles(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic, hasSRV, hasUAV);
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

void Scene::UpdateNrOfLigthsBuffer(ID3D11DeviceContext* context, bool shadowsOn)
{
	NrOfLights nrof;
	nrof.nrofDirLights = this->dirLights.GetNrOfLights();
	nrof.nrofSpotLights = this->spotLights.GetNrOfLights();
	nrof.shadowsOn = shadowsOn;
	this->nrofLights.UpdateBuffer(context, &nrof);
}

void Scene::UpdateObjects(ID3D11DeviceContext* context, float deltatime)
{
	for (auto dcem : dcems)
		dcem->Update(context);
	for (auto& object : objects)
		object->UpdateObject(context, deltatime);
}

void Scene::UpdateParticles(ID3D11DeviceContext* immediateContext, ShaderD3D11* pcs)
{
	UINT nrofParticles = particles->GetNrOfParticles();
	ID3D11Buffer* pBuffer = particles->GetParticlesBuffer();

	// Binding CS
	pcs->BindShader(immediateContext);

	ID3D11Buffer* pb = particles->GetParticlesBuffer();
	ID3D11UnorderedAccessView* puav = particles->GetUAV();
	immediateContext->CSSetConstantBuffers(0, 1, &pb);
	immediateContext->CSSetUnorderedAccessViews(0, 1, &puav, nullptr);

	// Dispath to CS
	immediateContext->Dispatch(std::ceil(nrofParticles / 32), 1, 1);

	// Unbind
	ID3D11UnorderedAccessView* uavNULL = nullptr;
	immediateContext->CSSetUnorderedAccessViews(0, 1, &uavNULL, nullptr);
	immediateContext->CSSetShader(nullptr, nullptr, 0);
}

void Scene::DrawScene(ID3D11DeviceContext* context, bool tesselate)
{
	// SHould probablu use this instead of many different funcs
}

void Scene::DrawObjects(ID3D11DeviceContext* immediatecontext, CameraD3D11* camera, bool tessellate)
{
	//int nrof = this->GetNrOfObjects();

	BoundingFrustum frustum;
	BoundingFrustum::CreateFromMatrix(frustum, camera->GetProjectionMatrix());
	XMMATRIX inverseView = XMMatrixInverse(nullptr, camera->GetViewMatrix());
	frustum.Transform(frustum, inverseView);
	std::vector<const Objects*> visibleObjs = quadtree.CheckTree(frustum);

	for (auto& obj : visibleObjs)
	{
		if (tessellate)
		{
			pCenter = obj->GetCenterBuffer();
			immediatecontext->HSSetConstantBuffers(1, 1, &pCenter);
		}
		obj->drawObject(immediatecontext);
	}
}

void Scene::GenerateDCEM(ID3D11DeviceContext* context)
{
	for (auto& dcem : dcems)
	{
		dcem->GenerateCubemap(context, objects);
	}
}

void Scene::DrawDCEM(ID3D11DeviceContext* context)
{
	for (auto& dcem : dcems)
	{
		dcem->Draw(context);
	}
}


void Scene::RemoveObjectFromScene(int index)
{
}

void Scene::DrawTree()
{
	quadtree.PrintTree();
}


ID3D11ShaderResourceView* Scene::GetLightBufferSRV(bool isDir)
{
	if (isDir) return this->dirLights.GetLightBufferSRV();
	return this->spotLights.GetLightBufferSRV();
}

ID3D11ShaderResourceView* Scene::GetShadowMapSRV(bool isDir)
{
	if (isDir) return this->dirLights.GetShadowMapsSRV();
	return this->spotLights.GetShadowMapsSRV();
}

ID3D11DepthStencilView* Scene::GetShadowMapDSV(int index, bool isDir)
{
	if (isDir) return this->dirLights.GetShadowMapDSV(index);
	return this->spotLights.GetShadowMapDSV(index);
}

XMFLOAT3 Scene::GetCameraPos(UINT lightIndex, bool isDir) const
{
	if (isDir) return this->dirLights.GetCameraPos(lightIndex);
	return this->spotLights.GetCameraPos(lightIndex);
}

XMFLOAT4X4 Scene::GetCameraVP(UINT lightINdex, bool isDir) const
{
	if (isDir) return this->dirLights.GetCameraVP(lightINdex);
	return this->spotLights.GetCameraVP(lightINdex);
}


ID3D11Buffer* Scene::GetShadowCamera(int index, bool isDir)
{
	if (isDir) return this->dirLights.GetLightCameraConstantBuffer(index, true);
	return this->spotLights.GetLightCameraConstantBuffer(index);
}

ID3D11Buffer* Scene::GetnrofLightBuffer() { return this->nrofLights.GetBuffer(); }

int Scene::GetNrOfObjects() { return (int)this->objects.size(); }

int Scene::GetNrOfSpotLights() { return this->spotLights.GetNrOfLights(); }

Particles* Scene::GetParticles() { return this->particles; }

int Scene::GetNrOfDirLight() { return this->dirLights.GetNrOfLights(); }
