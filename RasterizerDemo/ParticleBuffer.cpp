#include "ParticlesBuffer.h"

ParticleBuffer::ParticleBuffer(ID3D11Device* device, UINT sizeOfElement, UINT nrOfElementsInBuffer, void* bufferData, bool dynamic, bool hasSRV, bool hasUAV)
{
	this->Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic, hasSRV, hasUAV);
}

void ParticleBuffer::Initialize(ID3D11Device* device, UINT elementSize,
	UINT nrOfElements, void* bufferData, bool dynamic,
	bool hasSRV, bool hasUAV)
{
	// Dynamic is a bool, indicating if it should be possible to update buffer or not.

	this->elementSize = elementSize;
	this->nrOfElements = nrOfElements;
	this->dynamic = dynamic;
	

	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = elementSize * nrOfElements;
	desc.Usage = dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.BindFlags = hasSRV ? D3D11_BIND_SHADER_RESOURCE : 0;
	desc.BindFlags |= hasUAV ? D3D11_BIND_UNORDERED_ACCESS : 0;
	desc.CPUAccessFlags = dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementSize;

	HRESULT hr = S_OK;

	if (bufferData != nullptr)
	{
		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = &bufferData;
		data.SysMemPitch = 0; 
		data.SysMemSlicePitch = 0;

		hr = device->CreateBuffer(&desc, &data, this->buffer.GetAddressOf());
	}
	else
		hr = device->CreateBuffer(&desc, nullptr, this->buffer.GetAddressOf());

	if (FAILED(hr))
		throw std::runtime_error("Failed to create structured buffer");


	if (hasSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = nrOfElements;

		hr = device->CreateShaderResourceView(buffer.Get(), &srvDesc, &srv);

		if (FAILED(hr))
			throw std::runtime_error("Failed to create srv for structured buffer!");
	}

	if (hasUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = nrOfElements;
		uavDesc.Buffer.Flags = 0;

		hr = device->CreateUnorderedAccessView(buffer.Get(), &uavDesc, &uav);

		if (FAILED(hr))
			throw std::runtime_error("Failed to create uav for structured buffer!");
	}
}

UINT ParticleBuffer::GetElementSize() const { return this->elementSize; }
UINT ParticleBuffer::GetNrOfElements() const { return this->nrOfElements; }
ID3D11Buffer* ParticleBuffer::GetBuffer() const { return this->buffer.Get(); }
ID3D11ShaderResourceView* ParticleBuffer::GetSRV() const { return this->srv.Get(); }
ID3D11UnorderedAccessView* ParticleBuffer::GetUAV() const { return this->uav.Get(); }
