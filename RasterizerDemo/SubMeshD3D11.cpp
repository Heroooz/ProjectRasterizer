#include "SubMeshD3D11.h"

void SubMeshD3D11::Initialize(ID3D11Device* device, size_t startIndexValue, size_t nrOfIndicesInSubMesh,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ambientTextureSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTextureSRV,
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularTextureSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalTextureSRV,
    DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse, DirectX::XMFLOAT3 specular, float shinisess, float parallax)
{
    this->startIndex = startIndexValue;
    this->nrOfIndices = nrOfIndicesInSubMesh;
    this->ambientTexture = ambientTextureSRV;
    this->diffuseTexture = diffuseTextureSRV;
    this->specularTexture = specularTextureSRV;
    this->normalTexture = normalTextureSRV;

    MaterialBuffer material = {};
    material.ambientFactor = ambient;
    material.diffuseFactor = diffuse;
    material.specularFactor = specular;
    material.parallax = parallax;
    material.shininess = shinisess;
    material.hasAmbientTexture = ambientTextureSRV != nullptr;
    material.hasDiffuseTexture = diffuseTextureSRV != nullptr;
    material.hasSpecularTexture = specularTextureSRV != nullptr;
    material.hasNormalTexture = normalTextureSRV != nullptr;

    this->materialBuffer.Initialize(device, sizeof(MaterialBuffer), &material);
}

void SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext* context) const
{
    // Bind textures to the pixel shader
    context->PSSetShaderResources(0, 1, this->ambientTexture.GetAddressOf());
    context->PSSetShaderResources(1, 1, this->diffuseTexture.GetAddressOf());
    context->PSSetShaderResources(2, 1, this->specularTexture.GetAddressOf());
    context->PSSetShaderResources(3, 1, this->normalTexture.GetAddressOf());
    
    ID3D11Buffer* pBuffer = this->materialBuffer.GetBuffer();;
    context->PSSetConstantBuffers(1, 1, &pBuffer);

    // Perform the draw call
    context->DrawIndexed((INT)this->nrOfIndices, (UINT)this->startIndex, 0);
}

ID3D11ShaderResourceView* SubMeshD3D11::GetAmbientSRV() const { return this->ambientTexture.Get(); }

ID3D11ShaderResourceView* SubMeshD3D11::GetDiffuseSRV() const { return this->diffuseTexture.Get(); }

ID3D11ShaderResourceView* SubMeshD3D11::GetSpecularSRV() const { return this->specularTexture.Get(); }