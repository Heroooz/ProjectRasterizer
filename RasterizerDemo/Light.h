#pragma once
#include <d3d11.h>
#include "ConstantBufferD3D11.h"
#include <DirectXMath.h>

using namespace DirectX;

struct LightStruct
{
    XMFLOAT4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    XMFLOAT3 lightPosition = { 0.0f, 0.5f, -2.0f };
    float lightIntensity = 0.7f;
};
    //float shininess = 100.0f;

class Light
{
public:
	Light(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity);
	~Light();

    void Initialize(ID3D11Device* device, XMFLOAT4 color, XMFLOAT3 position, float intensity);

    void UpdateBuffer(ID3D11DeviceContext* context);
    ConstantBufferD3D11* GetBuffer();

    LightStruct GetLightStruct();

private:
    ConstantBufferD3D11 lightBuffer;
    LightStruct light;
};

