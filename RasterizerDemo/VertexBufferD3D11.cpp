#include "VertexBufferD3D11.h"
#include <stdexcept>

VertexBufferD3D11::VertexBufferD3D11(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData)
{
    Initialize(device, sizeOfVertex, nrOfVerticesInBuffer, vertexData);
}

VertexBufferD3D11::~VertexBufferD3D11()
{
}

void VertexBufferD3D11::Initialize(ID3D11Device* device, UINT sizeOfVertex, UINT nrOfVerticesInBuffer, void* vertexData)
{
    vertexSize = sizeOfVertex;
    nrOfVertices = nrOfVerticesInBuffer;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeOfVertex * nrOfVerticesInBuffer;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertexData;

    if (FAILED(device->CreateBuffer(&bufferDesc, &initData, &buffer)))
    {
        throw std::runtime_error("Failed to create vertex buffer");
    }
}

UINT VertexBufferD3D11::GetNrOfVertices() const
{
    return nrOfVertices;
}

UINT VertexBufferD3D11::GetVertexSize() const
{
    return vertexSize;
}

ID3D11Buffer* VertexBufferD3D11::GetBuffer() const
{
    return buffer.Get();
}
