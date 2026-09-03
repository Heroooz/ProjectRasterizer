#pragma once
#include <d3d11_4.h>
#include <DirectXMath.h>
#include <iostream>
#include <wrl/client.h>

using namespace DirectX;

class ParticleBuffer
{
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv = nullptr;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uav = nullptr;

	UINT elementSize = 0;
	UINT nrOfElements = 0;
	bool dynamic = false;

public:
	ParticleBuffer() = default;
	ParticleBuffer(ID3D11Device* device, UINT sizeOfElement, 
		UINT nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true,
		bool hasSRV = false, bool hasUAV = false);
	~ParticleBuffer() = default;
	ParticleBuffer(const ParticleBuffer& other) = delete;
	ParticleBuffer& operator=(const ParticleBuffer& other) = delete;
	ParticleBuffer(ParticleBuffer&& other) = delete;
	ParticleBuffer operator=(ParticleBuffer&& other) = delete;

	void Initialize(ID3D11Device* device, UINT sizeOfElement,
		UINT nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true,
		bool hasSRV = false, bool hasUAV = false);


	UINT GetElementSize() const;
	UINT GetNrOfElements() const;
	ID3D11Buffer* GetBuffer() const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11UnorderedAccessView* GetUAV() const;
};
