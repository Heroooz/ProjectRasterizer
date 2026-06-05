#pragma once

#include "Transform.h"
#include "Objects.h"
#include "Light.h"
#include "DCEM.h"
#include "ShaderD3D11.h"
#include "Particles.h"


class Scene
{
	std::vector<Objects*> objects;
	Light spotLights;
	Light dirLights;
	std::vector<DCEM*> dcems;

	std::vector<Particles*> particles;
	int nrofParticles;

	ConstantBufferD3D11 nrofLights;

	ID3D11Buffer* pCenter = nullptr;

public:
	Scene();
	~Scene();

	void Initialize();

	void AddDCEM(ID3D11Device* device, XMFLOAT3 position, UINT width, UINT height, std::unique_ptr<ShaderD3D11>& DCEMPS, std::unique_ptr<ShaderD3D11>& normalPS, bool isSkyBox);
	void AddObject(ID3D11Device* device, const std::string folderPath, const std::string objFile, XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, bool isStatic = true, float angle = 1.0f, bool SRT = true);
	//void AddLight(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity, bool isDir = false, float angle = 0.0f);
	void AddLight(ID3D11Device* device, LightData data);
	void AddParticles(ID3D11Device* device, UINT sizeOfElement, size_t nrOfElementsInBuffer,
		void* bufferData = nullptr, bool dynamic = true, bool hasSRV = false, bool hasUAV = false);


	void InitializeLight(ID3D11Device* device);
	void UpdateNrOfLigthsBuffer(ID3D11DeviceContext* context, bool shadowsOn);
	void UpdateObjects(ID3D11DeviceContext* context, float deltatime);

	void DrawScene(ID3D11DeviceContext* context);
	//void DrawLights(ID3D11DeviceContext* context);
	void DrawObjects(ID3D11DeviceContext* context, bool tesselate);
	void GenerateDCEM(ID3D11DeviceContext* context);
	void DrawDCEM(ID3D11DeviceContext* context);
	//void SetNrOfLights(ID3D11Device* device);
	void RemoveObjectFromScene(int index);

	ID3D11ShaderResourceView* GetLightBufferSRV(bool isDir = false);
	ID3D11ShaderResourceView* GetShadowMapSRV(bool isDir = false);
	ID3D11DepthStencilView* GetShadowMapDSV(int index, bool isDir = false);

	XMFLOAT3 GetCameraPos(UINT lightIndex, bool isDir=false) const;
	XMFLOAT4X4 GetCameraVP(UINT lightINdex, bool isDir=false) const;

	ID3D11Buffer* GetShadowCamera(int index, bool isDir = false);
	ID3D11Buffer* GetnrofLightBuffer();
	int GetNrOfObjects();
	int GetNrOfSpotLights();
	int GetNrOfDirLight();
};
