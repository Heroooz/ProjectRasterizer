#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>
#include "ShaderD3D11.h"



bool SetupPipeline(ID3D11Device* device, ShaderD3D11*& vShader, ShaderD3D11*& pShader, ShaderD3D11*& cShader, ID3D11InputLayout *& inputLayout,
	ID3D11Texture2D*& texture, ID3D11ShaderResourceView *& srv, ID3D11SamplerState *& sampler);
//bool SetupPipeline(ID3D11Device* device, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv, ID3D11SamplerState*& sampler);