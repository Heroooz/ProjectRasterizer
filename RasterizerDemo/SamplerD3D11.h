#pragma once

#include <optional> // Needs C++ 17 or higher
#include <array>
#include <iostream>

#include <d3d11_4.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

class SamplerD3D11
{
private:
	ComPtr<ID3D11SamplerState> sampler = nullptr;

public:
	SamplerD3D11() = default;
	SamplerD3D11(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode,
		std::optional<std::array<float, 4>> borderColour = std::nullopt, D3D11_SAMPLER_DESC sd = {});
	~SamplerD3D11() = default;
	SamplerD3D11(const SamplerD3D11& other) = delete;
	SamplerD3D11& operator=(const SamplerD3D11& other) = delete;
	SamplerD3D11(SamplerD3D11&& other) = delete;
	SamplerD3D11& operator=(SamplerD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, D3D11_TEXTURE_ADDRESS_MODE adressMode,
		std::optional<std::array<float, 4>> borderColour = std::nullopt, D3D11_SAMPLER_DESC sd = {});

	ID3D11SamplerState* GetSamplerState() const;
};