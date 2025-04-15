#include "MeshD3D11.h"

void MeshD3D11::Initialize(ID3D11Device* device, const MeshData& meshInfo)
{
    // Initialize vertex buffer
    vertexBuffer.Initialize(device, meshInfo.vertexInfo.sizeOfVertex, meshInfo.vertexInfo.nrOfVerticesInBuffer, meshInfo.vertexInfo.vertexData);

    // Initialize index buffer
    indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData);

    // Initialize sub-meshes
    subMeshes.resize(meshInfo.subMeshInfo.size());
    for (size_t i = 0; i < meshInfo.subMeshInfo.size(); ++i)
    {
        subMeshes[i].Initialize(
            meshInfo.subMeshInfo[i].startIndexValue,
            meshInfo.subMeshInfo[i].nrOfIndicesInSubMesh,
            meshInfo.subMeshInfo[i].ambientTextureSRV,
            meshInfo.subMeshInfo[i].diffuseTextureSRV,
            meshInfo.subMeshInfo[i].specularTextureSRV
        );
    }
}

void MeshD3D11::BindMeshBuffers(ID3D11DeviceContext* context) const
{
    // Bind vertex buffer
    UINT stride = static_cast<UINT>(vertexBuffer.GetNrOfVertices());
    UINT offset = 0;
    ID3D11Buffer* vb = vertexBuffer.GetBuffer();
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);

    // Bind index buffer
    context->IASetIndexBuffer(indexBuffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
}

void MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext* context, size_t subMeshIndex) const
{
    if (subMeshIndex < subMeshes.size())
    {
        subMeshes[subMeshIndex].PerformDrawCall(context);
    }
}

size_t MeshD3D11::GetNrOfSubMeshes() const
{
    return subMeshes.size();
}

ID3D11ShaderResourceView* MeshD3D11::GetAmbientSRV(size_t subMeshIndex) const
{
    if (subMeshIndex < subMeshes.size())
    {
        return subMeshes[subMeshIndex].GetAmbientSRV();
    }
    return nullptr;
}

ID3D11ShaderResourceView* MeshD3D11::GetDiffuseSRV(size_t subMeshIndex) const
{
    if (subMeshIndex < subMeshes.size())
    {
        return subMeshes[subMeshIndex].GetDiffuseSRV();
    }
    return nullptr;
}

ID3D11ShaderResourceView* MeshD3D11::GetSpecularSRV(size_t subMeshIndex) const
{
    if (subMeshIndex < subMeshes.size())
    {
        return subMeshes[subMeshIndex].GetSpecularSRV();
    }
    return nullptr;
}