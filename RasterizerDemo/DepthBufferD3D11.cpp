#include "DepthBufferD3D11.h"
#include <stdexcept>

#include <string>

DepthBufferD3D11::DepthBufferD3D11(ID3D11Device* device, UINT width, UINT height, bool hasSRV)
{
    Initialize(device, width, height, hasSRV);
}

//DepthBufferD3D11::~DepthBufferD3D11()
//{
//    if (this->texture)
//    {
//        texture->Release();
//        texture = nullptr;
//    }
//    if (this->srv)
//    {
//        srv->Release();
//        srv = nullptr;
//    }
//    for (auto& dsv : depthStencilViews)
//    {
//        if (dsv)
//        {
//            dsv->Release();
//        }
//        depthStencilViews.clear();
//    }
//}

void DepthBufferD3D11::Initialize(ID3D11Device* device, UINT width, UINT height, bool hasSRV, UINT arraySize)
{
    //this->depthStencilViews.clear();

    // Texture DESC
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = arraySize;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | (hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    if (FAILED(device->CreateTexture2D(&desc, nullptr, &this->texture)))
    {
        throw std::runtime_error("Failed to create depth buffer texture");
    }


    // DSV Desc
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    dsvDesc.Texture2DArray.ArraySize = 1;
    dsvDesc.Flags = 0;



    //this->depthStencilViews.resize(arraySize);
    for (UINT i = 0; i < arraySize; ++i)
    {
        ComPtr<ID3D11DepthStencilView> dsv = {};
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        if (FAILED(device->CreateDepthStencilView(this->texture.Get(), &dsvDesc, dsv.GetAddressOf())))
        {
            throw std::range_error("Failed to create depth stencil view");
        }
        this->depthStencilViews.push_back(dsv);
    }


    if (hasSRV)
    {
        // SRV Desc
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

        srvDesc.Texture2DArray.MostDetailedMip = 0;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize = arraySize;

        HRESULT hr = device->CreateShaderResourceView(this->texture.Get(), &srvDesc, this->srv.GetAddressOf());

        if (FAILED(hr))
        {
            throw std::runtime_error("Failed to create texture reasource view!" + std::to_string(hr));
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
    return this->srv.Get();
}
