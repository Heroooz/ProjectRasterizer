#include "Light.h"


Light::~Light()
{
	for (auto& camera : shadowCameras)
	{
		if (camera)
		{
			delete camera;
			camera = nullptr;
		}
	}
}

void Light::Initialize(ID3D11Device* device, const LightData& lightInfo)
{
	ID3D11DeviceContext* context;
	device->GetImmediateContext(&context);

	ProjectionInfo projInfo = {};
	projInfo.aspectRatio = lightInfo.perLightInfo.aspectRatio;
	projInfo.fovAngleY = lightInfo.perLightInfo.fovAngleY;
	projInfo.nearZ = lightInfo.perLightInfo.nearZ;
	projInfo.farZ = lightInfo.perLightInfo.farZ;

	CameraD3D11* lightCam = new CameraD3D11(device, projInfo, lightInfo.perLightInfo.initialPosition);
	lightCam->RotateUp(lightInfo.perLightInfo.rotationX);
	lightCam->RotateRight(lightInfo.perLightInfo.rotationY);


	LightBuffer light = {};
	light.color = lightInfo.perLightInfo.color;
	light.position = lightInfo.perLightInfo.initialPosition;
	light.angle = lightInfo.perLightInfo.angle;
	light.intensity = lightInfo.perLightInfo.intensity;

	if (lightInfo.perLightInfo.isDir)
	{
		this->isDirectionalLight = true;
		lightCam->UpdateOrthographicBuffer(context);
		light.vpmatrix = lightCam->GetOrthographicProjectionMatrix();
		light.direction = {-lightCam->GetForward().x, -lightCam->GetForward().y, -lightCam->GetForward().z };
	}
	else
	{
		lightCam->UpdateInternalConstantBuffer(context);
		light.vpmatrix = lightCam->GetViewProjectionMatrix();
		light.direction = lightCam->GetForward();
	}

	this->vpm = light.vpmatrix;
	this->pos = light.position;


	bufferData.push_back(light);
	shadowCameras.push_back(lightCam);

	context->Release();
}

void Light::InitializeBuffers(ID3D11Device* device)
{
	if (this->bufferData.size() > 0)
	{
		this->lightBuffer.Initialize(device, sizeof(LightBuffer), this->bufferData.size(), bufferData.data());
		this->shadowMaps.Initialize(device, WIDTH, HEIGHT, true, this->GetNrOfLights());
	}
}

void Light::UpdateBuffers(ID3D11DeviceContext* context)
{
	for (int i = 0; i < this->bufferData.size(); i++)
	{
		if (this->isDirectionalLight)
			bufferData[i].vpmatrix = shadowCameras[i]->GetOrthographicProjectionMatrix();
		else
			bufferData[i].vpmatrix = shadowCameras[i]->GetViewProjectionMatrix();
		
		this->lightBuffer.UpdateBuffer(context, &this->bufferData);
	}
}

UINT Light::GetNrOfLights() const { return (UINT)this->lightBuffer.GetNrOfElements(); }
ID3D11DepthStencilView* Light::GetShadowMapDSV(UINT lightIndex) const { return this->shadowMaps.GetDSV(lightIndex); }
ID3D11ShaderResourceView* Light::GetShadowMapsSRV() const { return this->shadowMaps.GetSRV(); }
ID3D11ShaderResourceView* Light::GetLightBufferSRV() const { return this->lightBuffer.GetSRV(); }

ID3D11Buffer* Light::GetLightCameraConstantBuffer(UINT lightIndex, bool orthographic) const 
{ 
	if (orthographic) return this->shadowCameras.at(lightIndex)->GetOrthographicConstantBuffer();
	return this->shadowCameras.at(lightIndex)->GetConstantBuffer(); 
}
XMFLOAT3 Light::GetCameraPos(UINT lightIndex) const { return this->pos; }
XMFLOAT4X4 Light::GetCameraVP(UINT lightINdex) const 
{ 
	if(isDirectionalLight) return this->shadowCameras.at(lightINdex)->GetOrthographicProjectionMatrix();
	return this->shadowCameras.at(lightINdex)->GetViewProjectionMatrix();
}

ID3D11Buffer* Light::GetLightBuffer() const { return this->lightBuffer.GetBuffer(); }
