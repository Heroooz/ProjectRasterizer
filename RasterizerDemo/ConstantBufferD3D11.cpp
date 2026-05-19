#include "ConstantBufferD3D11.h"
#include <iostream>

ConstantBufferD3D11::ConstantBufferD3D11(ID3D11Device* device, size_t byteSize, void* initialData)
{
    Initialize(device, byteSize, initialData);
}

ConstantBufferD3D11::~ConstantBufferD3D11()
{
}

ConstantBufferD3D11::ConstantBufferD3D11(ConstantBufferD3D11&& other) noexcept
    : buffer(std::move(other.buffer)), bufferSize(other.bufferSize)
{
    other.bufferSize = 0;
}

ConstantBufferD3D11& ConstantBufferD3D11::operator=(ConstantBufferD3D11&& other) noexcept
{
    if (this != &other)
    {
        buffer = std::move(other.buffer);
        bufferSize = other.bufferSize;
        other.bufferSize = 0;
    }
    return *this;
}

void ConstantBufferD3D11::Initialize(ID3D11Device* device, size_t byteSize, void* initialData)
{
    this->bufferSize = byteSize;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(byteSize);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = initialData;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &this->buffer);
    if (FAILED(hr))
    {
        std::cerr << "Failed to create constant buffer" << std::endl;
    }
}

size_t ConstantBufferD3D11::GetSize() const { return this->bufferSize; }

ID3D11Buffer* ConstantBufferD3D11::GetBuffer() const { return this->buffer; }

void ConstantBufferD3D11::UpdateBuffer(ID3D11DeviceContext* context, void* data)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ZeroMemory(&mappedResource, sizeof(mappedResource));
    HRESULT hr = context->Map(this->buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        std::cout << "Failed to map constant buffer" << std::endl;
    }

    memcpy(mappedResource.pData, data, this->bufferSize);
    context->Unmap(this->buffer, 0);
}
