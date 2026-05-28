#pragma once
#include <array>

struct SimpleVertex
{
	float pos[3];
	float normal[3];
	float uv[2];

	SimpleVertex(const std::array<float, 3>& position, const std::array<float, 3>& normal, const std::array<float, 2>& uv)
	{
		for (int i = 0; i < 3; ++i)
		{
			this->pos[i] = position[i];
			this->normal[i] = normal[i];
		}

		this->uv[0] = uv[0];
		this->uv[1] = uv[1];
	}
};