#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"

using namespace DirectX;

struct LightData
{
	struct PerLightInfo
	{
		DirectX::XMFLOAT4 color = { 0, 0, 0, 0};
		DirectX::XMFLOAT3 initialPosition = { 0, 0 ,0 };
		bool isDir = false;
		float angle = 0.0f;
		float intensity = 0.0f;
		float rotationX = 0.0f;
		float rotationY = 0.0f;
		float fovAngleY = 1.0f;
		float aspectRatio = 16 / 9;
		float nearZ = 0.1f;
		float farZ = 100;
	} perLightInfo;
};

struct NrOfLights
{
	int nrofSpotLights = 0;
	int nrofDirLights = 0;
	int shadowsOn = 0;
	int padding1 = 0;
};

class Light
{
	struct LightBuffer
	{
		XMFLOAT4X4 vpmatrix;
		XMFLOAT4 color;
		XMFLOAT3 position;
		float intensity;
		XMFLOAT3 direction;
		float angle;
	};

	std::vector<LightBuffer> bufferData;
	std::vector<CameraD3D11*> shadowCameras;

	DepthBufferD3D11 shadowMaps;
	StructuredBufferD3D11 lightBuffer;

	bool isDirectionalLight = false;
	XMFLOAT3 pos;
	XMFLOAT4X4 vpm;

	const UINT WIDTH = 1920;
	const UINT HEIGHT = 1080;

public:
	Light() = default;
	//Light(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity);
	~Light();
	Light(const Light& other) = delete;
	Light& operator=(const Light& other) = delete;
	Light(Light&& other) = delete;
	Light& operator=(DepthBufferD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, const LightData& lightInfo);
	void InitializeBuffers(ID3D11Device* device);
    //void InitiaLightlize(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity);
    void UpdateBuffers(ID3D11DeviceContext* context);

	UINT GetNrOfLights() const;
	ID3D11DepthStencilView* GetShadowMapDSV(UINT lightIndex) const;
	ID3D11ShaderResourceView* GetShadowMapsSRV() const;
	ID3D11ShaderResourceView* GetLightBufferSRV() const;
	XMFLOAT3 GetCameraPos(UINT lightIndex) const;
	XMFLOAT4X4 GetCameraVP(UINT lightINdex) const;
	ID3D11Buffer* GetLightCameraConstantBuffer(UINT lightIndex, bool orthographic = false) const;
	ID3D11Buffer* GetLightBuffer() const;
};
