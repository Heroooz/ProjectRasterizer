#include "DepthBufferD3D11.h"
#include <stdexcept>

DepthBufferD3D11::DepthBufferD3D11(ID3D11Device* device, UINT width, UINT height, bool hasSRV)
{
    Initialize(device, width, height, hasSRV);
}

DepthBufferD3D11::~DepthBufferD3D11()
{
}

void DepthBufferD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, bool hasSRV, UINT arraySize)
{
    depthStencilViews.clear();

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = arraySize;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | (hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    if (FAILED(device->CreateTexture2D(&desc, nullptr, &texture)))
    {
        throw std::runtime_error("Failed to create depth buffer texture");
    }

    depthStencilViews.resize(arraySize);
    for (UINT i = 0; i < arraySize; ++i)
    {
        if (FAILED(device->CreateDepthStencilView(texture.Get(), nullptr, &depthStencilViews[i])))
        {
            throw std::range_error("Failed to create depth stencil view");
        }
    }

    if (hasSRV)
    {
        if (FAILED(device->CreateShaderResourceView(texture.Get(), 0, &srv)))
        {
            throw std::runtime_error("Failed to create texture reasource view!");
        }
    }
}

ID3D11DepthStencilView* DepthBufferD3D11::GetDSV(UINT arrayIndex) const
{
    if (arrayIndex >= depthStencilViews.size())
    {
        throw std::runtime_error("Array index out of range");
    }
    return depthStencilViews[arrayIndex].Get();
}

ID3D11ShaderResourceView* DepthBufferD3D11::GetSRV() const
{
    return srv.Get();
}
