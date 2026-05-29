#include "GBuffer.h"

GBuffer::GBuffer(ID3D11Device* device, UINT width, UINT height)
{
	Initialize(device, width, height);
}

GBuffer::~GBuffer()
{
}

void GBuffer::Initialize(ID3D11Device* device, UINT width, UINT height)
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
		std::cerr << "Failed to create G-buffer texture" << std::endl;
		return;
	}

	hr = device->CreateShaderResourceView(this->texture, nullptr, &this->srv);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create G-buffer srv" << std::endl;
		return;
	}

	hr = device->CreateRenderTargetView(this->texture, nullptr, &this->rtv);
	if (FAILED(hr))
	{
		std::cerr << "Failed to create G-buffer rtv" << std::endl;
		return;
	}
}

ID3D11Texture2D* GBuffer::GetTexture() const { return this->texture; }

ID3D11ShaderResourceView* GBuffer::GetSRV() const { return this->srv; }

ID3D11RenderTargetView* GBuffer::GetRTV() const { return this->rtv; }
