#include "SubMeshD3D11.h"

void SubMeshD3D11::Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh,
    ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* diffuseTextureSRV,
    ID3D11ShaderResourceView* specularTextureSRV, ID3D11ShaderResourceView* bumpTextureSRV)
{
    this->startIndex = startIndexValue;
    this->nrOfIndices = nrOfIndicesInSubMesh;
    this->ambientTexture = ambientTextureSRV;
    this->diffuseTexture = diffuseTextureSRV;
    this->specularTexture = specularTextureSRV;
    this->bumpTexture = bumpTextureSRV;
}

void SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext* context) const
{
    // Bind textures to the pixel shader
    context->PSSetShaderResources(0, 1, &ambientTexture);
    context->PSSetShaderResources(1, 1, &diffuseTexture);
    context->PSSetShaderResources(2, 1, &specularTexture);

    if (this->bumpTexture != nullptr)
        context->PSSetShaderResources(3, 1, &bumpTexture);

    // Perform the draw call
    context->DrawIndexed((INT)this->nrOfIndices, (UINT)this->startIndex, 0);
}

ID3D11ShaderResourceView* SubMeshD3D11::GetAmbientSRV() const { return this->ambientTexture; }

ID3D11ShaderResourceView* SubMeshD3D11::GetDiffuseSRV() const { return this->diffuseTexture; }

ID3D11ShaderResourceView* SubMeshD3D11::GetSpecularSRV() const { return this->specularTexture; }