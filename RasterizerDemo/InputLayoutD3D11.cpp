#include "InputLayoutD3D11.h"

InputLayoutD3D11::~InputLayoutD3D11()
{
}

void InputLayoutD3D11::AddInputElement(const std::string& semanticName, DXGI_FORMAT format)
{
}

void InputLayoutD3D11::FinalizeInputLayout(ID3D11Device* device, const void* vsDataPtr, size_t vsDataSize)
{
}

ID3D11InputLayout* InputLayoutD3D11::GetInputLayout() const
{
    return nullptr;
}
