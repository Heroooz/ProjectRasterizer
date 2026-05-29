#include "ShaderResourceTextureD3D11.h"

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, UINT width, UINT height, void* textureData)
{
	Initialize(device, width, height, textureData);
}

ShaderResourceTextureD3D11::ShaderResourceTextureD3D11(ID3D11Device* device, const char* pathToTextureFile)
{
	Initialize(device, pathToTextureFile);
}

ShaderResourceTextureD3D11::~ShaderResourceTextureD3D11()
{
}

void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, void* textureData)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &this->texture);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create G-buffer texture");
	}

	hr = device->CreateShaderResourceView(texture, nullptr, &this->srv);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create G-buffer srv");
	}

	ID3D11RenderTargetView* rtv = nullptr;
	hr = device->CreateRenderTargetView(texture, nullptr, &rtv);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create G-buffer rtv");
	}
}

void ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, const char* pathToTextureFile)
{

}

ID3D11ShaderResourceView* ShaderResourceTextureD3D11::GetSRV() const
{
	return nullptr;
}
