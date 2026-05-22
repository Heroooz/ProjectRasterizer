#pragma once
#include <d3d11_4.h>
#include <DirectXMath.h>

struct Transform
{
	float position[3];
	float rotation[3];
	float scale[3];

	Transform(const std::array<float, 3>& position = { 0.0f, 0.0f, 0.0f }, const std::array<float, 3>& rotation = { 0.0f, 0.0f, 0.0f }, const std::array<float, 3>& scale = { 1.0f, 1.0f, 1.0f })
	{
		for (size_t i = 0; i < 3; i++)
		{
			this->position[i] = position[i];
			this->rotation[i] = rotation[i];
			this->scale[i] = scale[i];
		}
	}
};