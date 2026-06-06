#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>
#include "ShaderD3D11.h"
#include "InputLayoutD3D11.h"
#include "SamplerD3D11.h"
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;



bool SetupPipeline(ComPtr<ID3D11Device> device, std::unique_ptr<ShaderD3D11>& vShader, std::unique_ptr<ShaderD3D11>& deferredPShader, std::unique_ptr<ShaderD3D11>& pShader, std::unique_ptr<ShaderD3D11>& DCEMCapturePS,
	std::unique_ptr<ShaderD3D11>& cShader, std::unique_ptr<ShaderD3D11>& hullShader, std::unique_ptr<ShaderD3D11>& domainShader, std::unique_ptr<ShaderD3D11>& particleShadersVS,
	std::unique_ptr<ShaderD3D11>& particleShadersGS, std::unique_ptr<ShaderD3D11>& particleShadersPS, std::unique_ptr<ShaderD3D11>& particleShadersCS,
	std::unique_ptr<InputLayoutD3D11>& inputLayout, ComPtr<ID3D11Texture2D>& texture, ComPtr<ID3D11ShaderResourceView>& srv, std::unique_ptr<SamplerD3D11>& samplerState,
	std::unique_ptr<SamplerD3D11>& shadowSampler);
//bool SetupPipeline(ID3D11Device* device, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv, ID3D11SamplerState*& sampler);