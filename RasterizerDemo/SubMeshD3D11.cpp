#include "SubMeshD3D11.h"

void SubMeshD3D11::Initialize(ID3D11Device* device, size_t startIndexValue, size_t nrOfIndicesInSubMesh,
    ID3D11ShaderResourceView* ambientTextureSRV, ID3D11ShaderResourceView* diffuseTextureSRV,
    ID3D11ShaderResourceView* specularTextureSRV, ID3D11ShaderResourceView* bumpTextureSRV, 
    ID3D11ShaderResourceView* normalTexture, DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse,
    DirectX::XMFLOAT3 specular, float shinisess)
{
    this->startIndex = startIndexValue;
    this->nrOfIndices = nrOfIndicesInSubMesh;
    this->ambientTexture = ambientTextureSRV;
    this->diffuseTexture = diffuseTextureSRV;
    this->specularTexture = specularTextureSRV;
    this->normalTexture = normalTexture;
    this->bumpTexture = bumpTextureSRV;

    MaterialBuffer material = {};
    material.ambientFactor = ambient;
    material.diffuseFactor = diffuse;
    material.specularFactor = specular;
    material.shininess = shinisess;

    this->materialBuffer.Initialize(device, sizeof(MaterialBuffer), &material);
}

void SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext* context) const
{
    // Bind textures to the pixel shader
    context->PSSetShaderResources(0, 1, &ambientTexture);
    context->PSSetShaderResources(1, 1, &diffuseTexture);
    context->PSSetShaderResources(2, 1, &specularTexture);

    //if (this->bumpTexture != nullptr)
    //    context->PSSetShaderResources(3, 1, &bumpTexture);
    //if (this->normalTexture != nullptr)
    //    context->PSSetShaderResources(4, 1, &normalTexture);
    
    ID3D11Buffer* pBuffer = this->materialBuffer.GetBuffer();
    context->PSSetConstantBuffers(2, 1, &pBuffer);

    // Perform the draw call
    context->DrawIndexed((INT)this->nrOfIndices, (UINT)this->startIndex, 0);
}

ID3D11ShaderResourceView* SubMeshD3D11::GetAmbientSRV() const { return this->ambientTexture; }

ID3D11ShaderResourceView* SubMeshD3D11::GetDiffuseSRV() const { return this->diffuseTexture; }

ID3D11ShaderResourceView* SubMeshD3D11::GetSpecularSRV() const { return this->specularTexture; }