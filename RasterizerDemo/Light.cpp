#include "Light.h"


Light::~Light()
{
	for (auto& camera : shadowCameras)
	{
		delete camera;
		camera = nullptr;
	}
}

void Light::Initialize(ID3D11Device* device, const LightData& lightInfo)
{
	CameraD3D11* lightCam = new CameraD3D11();
	ProjectionInfo projInfo = {};
	projInfo.aspectRatio = lightInfo.perLightInfo.aspectRatio;
	projInfo.fovAngleY = lightInfo.perLightInfo.fovAngleY;
	projInfo.nearZ = lightInfo.perLightInfo.nearZ;
	projInfo.farZ = lightInfo.perLightInfo.farZ;

	lightCam->Initialize(device, projInfo, lightInfo.perLightInfo.initialPosition);
	lightCam->RotateUp(lightInfo.perLightInfo.rotationX);
	lightCam->RotateRight(lightInfo.perLightInfo.rotationY);

	ID3D11DeviceContext* context;
	device->GetImmediateContext(&context);
	lightCam->UpdateInternalConstantBuffer(context);


	LightBuffer light = {};
	light.color = lightInfo.perLightInfo.color;
	light.position = lightInfo.perLightInfo.initialPosition;
	light.angle = lightInfo.perLightInfo.angle;
	light.intensity = lightInfo.perLightInfo.intensity;
	light.vpmatrix = lightCam->GetViewProjectionMatrix();
	light.direction = lightCam->GetForward();
	
	bufferData.push_back(light);
	shadowCameras.push_back(lightCam);

	context->Release();
}

void Light::InitializeBuffers(ID3D11Device* device)
{
	this->lightBuffer.Initialize(device, sizeof(LightBuffer), this->bufferData.size(), bufferData.data());
	this->shadowMaps.Initialize(device, 1024, 512, false, this->GetNrOfLights());
}

void Light::UpdateBuffers(ID3D11DeviceContext* context)
{
	this->lightBuffer.UpdateBuffer(context, &this->bufferData);
}

UINT Light::GetNrOfLights() const { return this->lightBuffer.GetNrOfElements(); }
ID3D11DepthStencilView* Light::GetShadowMapDSV(UINT lightIndex) const { return this->shadowMaps.GetDSV(lightIndex); }
ID3D11ShaderResourceView* Light::GetShadowMapsSRV() const { return this->shadowMaps.GetSRV(); }
ID3D11ShaderResourceView* Light::GetLightBufferSRV() const { return this->lightBuffer.GetSRV(); }
ID3D11Buffer* Light::GetLightCameraConstantBuffer(UINT lightIndex) const { return this->shadowCameras.at(lightIndex)->GetConstantBuffer(); };
ID3D11Buffer* Light::GetLightBuffer() const { return this->lightBuffer.GetBuffer(); }


//Light::Light(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity)
//{
//    Initialize(device, color, position, intensity);
////}
//
//Light::~Light()
//{
//}

//void Light::Initialize(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity)
//{
//    this->light.lightColor = color;
//    this->light.lightPosition = position;
//    this->light.lightIntensity = intensity;
//
//    this->lightBuffer.Initialize(device, sizeof(LightStruct), &this->light);
//}
//
//void Light::UpdateBuffer(ID3D11DeviceContext* context)
//{
//    this->lightBuffer.UpdateBuffer(context, &this->light);
//}
//
//ConstantBufferD3D11* Light::GetBuffer() { return &this->lightBuffer; }
//
//LightStruct Light::GetLightStruct() { return this->light; }
