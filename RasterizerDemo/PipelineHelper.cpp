#include "PipelineHelper.h"

#include <fstream>
#include <string>
#include <iostream>
#include <optional>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool LoadShaders(ComPtr<ID3D11Device> device, std::unique_ptr<ShaderD3D11>& vShader, std::unique_ptr<ShaderD3D11>& defpShader, 
	std::unique_ptr<ShaderD3D11>& pShader, std::unique_ptr<ShaderD3D11>& DCEMCapturePS, std::unique_ptr<ShaderD3D11>& cShader, std::unique_ptr<ShaderD3D11>& hullShader,
	std::unique_ptr<ShaderD3D11>& domainShader, std::unique_ptr<ShaderD3D11>& particleShadersVS, std::unique_ptr<ShaderD3D11>& particleShadersPS,
	std::unique_ptr<ShaderD3D11>& particleShadersGS, std::unique_ptr<ShaderD3D11>& particleShadersCS)
{

	vShader = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::VERTEX_SHADER, "VertexShader.cso");
	defpShader = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::PIXEL_SHADER, "DeferredPS.cso");
	pShader = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::PIXEL_SHADER, "DCEMPS.cso");
	DCEMCapturePS = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::PIXEL_SHADER, "DCEMCapturePS.cso");
	cShader = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::COMPUTE_SHADER, "ComputeShader.cso");

	hullShader = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::HULL_SHADER, "HullShader.cso");
	domainShader = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::DOMAIN_SHADER, "DomainShader.cso");


	particleShadersVS = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::VERTEX_SHADER, "ParticleVS.cso");
	particleShadersGS = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::PIXEL_SHADER, "ParticlePS.cso");
	particleShadersPS = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::GEOMETRY_SHADER, "ParticleGS.cso");
	particleShadersCS = std::make_unique<ShaderD3D11>(device.Get(), ShaderType::COMPUTE_SHADER, "ParticleCS.cso");

	return true;
}

bool CreateInputLayout(ComPtr<ID3D11Device> device, std::unique_ptr<InputLayoutD3D11>& inputLayout, std::unique_ptr<ShaderD3D11>& vShader)
{
	inputLayout = std::make_unique<InputLayoutD3D11>();

	size_t inputSlot = 0;
	inputLayout->AddInputElement("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, inputSlot);
	inputSlot += 12;
	inputLayout->AddInputElement("NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, inputSlot);
	inputSlot += 12;
	inputLayout->AddInputElement("UV", DXGI_FORMAT_R32G32_FLOAT, inputSlot);

	inputLayout->FinalizeInputLayout(device.Get(), vShader->GetShaderByteData(), vShader->GetShaderByteSize());

	return true;
}

bool CreateTexture(ComPtr<ID3D11Device> device, const char* filename, int x, int y, int comp,
	ComPtr<ID3D11Texture2D>& texture, ComPtr<ID3D11ShaderResourceView>& srv) {
	stbi__vertically_flip_on_load = true;
	unsigned char* textureData = stbi_load(filename, &x, &y, &comp, 4);
	if (textureData == nullptr) {
		std::cerr << "Failed to load texture!" << std::endl;
		return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = x;
	textureDesc.Height = y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = textureData;
	data.SysMemPitch = x * 4;
	data.SysMemSlicePitch = 0;

	if (FAILED(device->CreateTexture2D(&textureDesc, &data, texture.GetAddressOf()))) {
		std::cerr << "Failed to create texture!" << std::endl;
		return false;
	}

	if (FAILED(device->CreateShaderResourceView(texture.Get(), nullptr, srv.GetAddressOf()))) {
		std::cerr << "Failed to create texture reasource view!" << std::endl;
		return false;
	}

	stbi_image_free(textureData);
	return true;
}

bool CreateSamplerState(ComPtr<ID3D11Device> device, std::unique_ptr<SamplerD3D11>& samplerState, std::unique_ptr<SamplerD3D11>& shadowSampler)
{
	D3D11_TEXTURE_ADDRESS_MODE addressmode = D3D11_TEXTURE_ADDRESS_WRAP;
	std::optional<std::array<float, 4>> borderColour;
	borderColour = { 1,1,1,1 };
	samplerState = std::make_unique<SamplerD3D11>(device.Get(), addressmode, borderColour);


	D3D11_SAMPLER_DESC sd = {};
	sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.MipLODBias = 0.0f;
	sd.MaxAnisotropy = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sd.MinLOD = 0.0f;
	sd.MaxLOD = D3D11_FLOAT32_MAX;


	shadowSampler = std::make_unique<SamplerD3D11>(device.Get(), D3D11_TEXTURE_ADDRESS_CLAMP, borderColour, sd);	

	return true;
}

bool SetupPipeline(ComPtr<ID3D11Device> device, std::unique_ptr<ShaderD3D11>& vShader, std::unique_ptr<ShaderD3D11>& deferredPShader, std::unique_ptr<ShaderD3D11>& pShader, std::unique_ptr<ShaderD3D11>& DCEMCapturePS,
	std::unique_ptr<ShaderD3D11>& cShader, std::unique_ptr<ShaderD3D11>& hullShader, std::unique_ptr<ShaderD3D11>& domainShader, std::unique_ptr<ShaderD3D11>& particleShadersVS,
	std::unique_ptr<ShaderD3D11>& particleShadersGS, std::unique_ptr<ShaderD3D11>& particleShadersPS, std::unique_ptr<ShaderD3D11>& particleShadersCS,
	std::unique_ptr<InputLayoutD3D11>& inputLayout, ComPtr<ID3D11Texture2D>& texture, ComPtr<ID3D11ShaderResourceView>& srv, std::unique_ptr<SamplerD3D11>& samplerState,
	std::unique_ptr<SamplerD3D11>& shadowSampler)
{
	if (!LoadShaders(device, vShader, deferredPShader, pShader, DCEMCapturePS, cShader, hullShader, domainShader, particleShadersVS, particleShadersPS, particleShadersGS, particleShadersCS))
	{
		std::cerr << "Error loading shaders!" << std::endl;
		return false;
	}
	
	if (!CreateInputLayout(device, inputLayout, vShader))
	{
		std::cerr << "Error creating input layout!" << std::endl;
		return false;
	}

	if (!CreateTexture(device, "texture.jpg", 900, 600, 0, texture, srv)) {
		std::cerr << "Error creating texture!" << std::endl;
		return false;
	}

	if (!CreateSamplerState(device, samplerState, shadowSampler)) {
		std::cerr << "Error creating sampler!" << std::endl;
		return false;
	}

	return true;
}
