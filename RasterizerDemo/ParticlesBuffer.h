#pragma once
#include <d3d11_4.h>
//#include <d3d11.h>
#include <iostream>
#include <wrl/client.h>

//using namespace /*DirectX*/;

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
	size_t GetNrOfElements() const;
	ID3D11Buffer* GetBuffer() const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11UnorderedAccessView* GetUAV() const;
};

/*

public:
	StructuredBufferD3D11() = default;
	StructuredBufferD3D11(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true);
	~StructuredBufferD3D11() = default;
	StructuredBufferD3D11(const StructuredBufferD3D11& other) = delete;
	StructuredBufferD3D11& operator=(const StructuredBufferD3D11& other) = delete;
	StructuredBufferD3D11(StructuredBufferD3D11&& other) = delete;
	StructuredBufferD3D11 operator=(StructuredBufferD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true);

	void UpdateBuffer(ID3D11DeviceContext* context, void* data);

	UINT GetElementSize() const;
	size_t GetNrOfElements() const;
	ID3D11ShaderResourceView* GetSRV() const;
	ID3D11Buffer* GetBuffer() const;

*/