#include "IndexBufferD3D11.h"
#include <stdexcept>

IndexBufferD3D11::IndexBufferD3D11(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
    Initialize(device, nrOfIndicesInBuffer, indexData);
}

IndexBufferD3D11::~IndexBufferD3D11()
{
}

void IndexBufferD3D11::Initialize(ID3D11Device* device, size_t nrOfIndicesInBuffer, uint32_t* indexData)
{
    nrOfIndices = nrOfIndicesInBuffer;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = static_cast<UINT>(nrOfIndices * sizeof(uint32_t));
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indexData;

    HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, &buffer);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create index buffer");
    }
}

size_t IndexBufferD3D11::GetNrOfIndices() const
{
    return nrOfIndices;
}

ID3D11Buffer* IndexBufferD3D11::GetBuffer() const
{
    return buffer.Get();
}
