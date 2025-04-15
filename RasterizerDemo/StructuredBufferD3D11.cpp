#include "StructuredBufferD3D11.h"
#include <stdexcept>

StructuredBufferD3D11::StructuredBufferD3D11(ID3D11Device* device, UINT sizeOfElement,
    size_t nrOfElementsInBuffer, void* bufferData, bool dynamic)
{
    Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic);
}

StructuredBufferD3D11::~StructuredBufferD3D11()
{
}

void StructuredBufferD3D11::Initialize(ID3D11Device* device, UINT sizeOfElement,
    size_t nrOfElementsInBuffer, void* bufferData, bool dynamic)
{
    elementSize = sizeOfElement;
    nrOfElements = nrOfElementsInBuffer;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = static_cast<UINT>(elementSize * nrOfElements);
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = elementSize;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = bufferData;

    HRESULT hr = device->CreateBuffer(&bufferDesc, bufferData ? &initData : nullptr, &buffer);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create structured buffer");
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = static_cast<UINT>(nrOfElements);

    hr = device->CreateShaderResourceView(buffer.Get(), &srvDesc, &srv);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create shader resource view for structured buffer");
    }
}

void StructuredBufferD3D11::UpdateBuffer(ID3D11DeviceContext* context, void* data)
{
    if (!buffer)
    {
        throw std::runtime_error("Buffer not initialized");
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to map structured buffer");
    }

    memcpy(mappedResource.pData, data, elementSize * nrOfElements);
    context->Unmap(buffer.Get(), 0);
}

UINT StructuredBufferD3D11::GetElementSize() const
{
    return elementSize;
}

size_t StructuredBufferD3D11::GetNrOfElements() const
{
    return nrOfElements;
}

ID3D11ShaderResourceView* StructuredBufferD3D11::GetSRV() const
{
    return srv.Get();
}
