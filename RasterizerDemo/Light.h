#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

struct lightStruct
{
    DirectX::XMFLOAT4 lightPosition = { 0.0f, 0.5f, -2.0f, 0.0f };
    DirectX::XMFLOAT4 lightColour = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4 cameraPosition = { 0.0f, 0.0f, -2.0f, 1.0f };
    float lightIntensity = 0.1f;
    float shininess = 100.0f;
    char padding[8] = { ' ' };
};