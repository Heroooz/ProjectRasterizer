#include "RenderTargetD3D11.h"
#include <stdexcept>

RenderTargetD3D11::~RenderTargetD3D11()
{
}

void RenderTargetD3D11::Initialize(ID3D11Device* device, IDXGISwapChain* swapChain, UINT width, UINT height, DXGI_FORMAT format, bool hasSRV)
{
    ID3D11Texture2D* backBuffer = nullptr;
    if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
    {
        throw std::runtime_error("Failed to get swapchain buffer");
    }

    if (FAILED(device->CreateRenderTargetView(backBuffer, nullptr, &rtv)))
    {
        throw std::runtime_error("Failed to create render target view");
    }
	backBuffer->Release();

    if (hasSRV)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        if (FAILED(device->CreateShaderResourceView(texture.Get(), &srvDesc, &srv)))
        {
            throw std::runtime_error("Failed to create shader resource view");
        }
    }
}

ID3D11RenderTargetView* RenderTargetD3D11::GetRTV() const
{
    return this->rtv.Get();
}

ID3D11ShaderResourceView* RenderTargetD3D11::GetSRV() const
{
    return this->srv.Get();
}