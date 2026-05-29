#include "InputLayoutD3D11.h"

InputLayoutD3D11::~InputLayoutD3D11() 
{ 
	if (this->inputLayout)
	{
		this->inputLayout->Release(); 
		this->inputLayout = nullptr;
	}
}

void InputLayoutD3D11::AddInputElement(const char* semanticName, DXGI_FORMAT format, size_t inputSlot)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc =
	{ semanticName, 0, format, 0, inputSlot, D3D11_INPUT_PER_VERTEX_DATA, 0 };


    this->elements.push_back(inputDesc);
    this->semanticNames.push_back(this->elements.back().SemanticName);
}

void InputLayoutD3D11::FinalizeInputLayout(ID3D11Device* device, const void* vsDataPtr, size_t vsDataSize)
{
	if (FAILED(device->CreateInputLayout(this->elements.data(), this->elements.size(), vsDataPtr, vsDataSize, &this->inputLayout)))
	{
		std::cerr << "Failed to create Input Layout!" << std::endl;
		throw std::runtime_error("Failed to create Input Layout!");
	}
}

ID3D11InputLayout* InputLayoutD3D11::GetInputLayout() const { return this->inputLayout; }
