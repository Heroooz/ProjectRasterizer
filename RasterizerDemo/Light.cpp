#include "Light.h"

Light::Light(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity)
{
    Initialize(device, color, position, intensity);
}

Light::~Light()
{
}

void Light::Initialize(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity)
{
    this->light.lightColor = color;
    this->light.lightPosition = position;
    this->light.lightIntensity = intensity;

    this->lightBuffer.Initialize(device, sizeof(LightStruct), &this->light);
}

void Light::UpdateBuffer(ID3D11DeviceContext* context)
{
    this->lightBuffer.UpdateBuffer(context, &this->light);
}

ConstantBufferD3D11* Light::GetBuffer() { return &this->lightBuffer; }

LightStruct Light::GetLightStruct() { return this->light; }
