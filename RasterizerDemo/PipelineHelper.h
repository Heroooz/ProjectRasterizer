#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>



bool SetupPipeline(ID3D11Device* device, ID3D11VertexShader*& vShader, ID3D11PixelShader*& pShader, ID3D11InputLayout *& inputLayout,
	ID3D11Texture2D*& texture, ID3D11ShaderResourceView *& srv, ID3D11SamplerState *& sampler);
//bool SetupPipeline(ID3D11Device* device, ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv, ID3D11SamplerState*& sampler);