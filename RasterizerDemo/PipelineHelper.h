#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>
#include "ShaderD3D11.h"
#include "InputLayoutD3D11.h"



bool SetupPipeline(ID3D11Device* device, ID3D11DeviceContext* context, ShaderD3D11*& vShader, ShaderD3D11*& pShader, ShaderD3D11*& cShader, InputLayoutD3D11*& inputLayout,
	ID3D11Texture2D*& texture, ID3D11ShaderResourceView *& srv, ID3D11SamplerState *& sampler);
//bool SetupPipeline(ID3D11Device* device, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv, ID3D11SamplerState*& sampler);