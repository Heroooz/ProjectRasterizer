#pragma once

#include <string>
#include "ConstantBufferD3D11.h"
#include <d3d11_4.h>
#include <DirectXMath.h>

#include <wrl/client.h>

class SubMeshD3D11
{
private:

	struct MaterialBuffer
	{
		DirectX::XMFLOAT3 ambientFactor;
		float shininess = 100.0f;
		DirectX::XMFLOAT3 diffuseFactor;
		float parallax = 0.0f;
		DirectX::XMFLOAT3 specularFactor;
		float padding3 = 0.0f;
		int hasAmbientTexture = false;
		int hasDiffuseTexture = false;
		int hasSpecularTexture = false;
		int hasNormalTexture = false;
	};

	size_t startIndex = 0;
	size_t nrOfIndices = 0;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ambientTexture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTexture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularTexture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalTexture = nullptr;

	ConstantBufferD3D11 materialBuffer;

public:
	SubMeshD3D11() = default;
	~SubMeshD3D11() = default;
	SubMeshD3D11(const SubMeshD3D11& other) = default;
	SubMeshD3D11& operator=(const SubMeshD3D11& other) = default;
	SubMeshD3D11(SubMeshD3D11&& other) = default;
	SubMeshD3D11& operator=(SubMeshD3D11&& other) = default;

	void Initialize(ID3D11Device* device, size_t startIndexValue, size_t nrOfIndicesInSubMesh,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ambientTextureSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseTextureSRV,
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> specularTextureSRV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalTextureSRV,
		DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse, DirectX::XMFLOAT3 specular, float shinisess = 100.0f, float parallax = 0.0f);

	void PerformDrawCall(ID3D11DeviceContext* context) const;

	ID3D11ShaderResourceView* GetAmbientSRV() const;
	ID3D11ShaderResourceView* GetDiffuseSRV() const;
	ID3D11ShaderResourceView* GetSpecularSRV() const;
};