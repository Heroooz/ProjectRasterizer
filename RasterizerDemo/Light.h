#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

struct lightStruct
{
    XMFLOAT4 lightColour = {1.0f, 1.0f, 1.0f, 1.0f};
    XMFLOAT3 lightPosition = { 0.0f, 0.5f, -2.0f };
    float lightIntensity = 0.7f;
};
    //float shininess = 100.0f;