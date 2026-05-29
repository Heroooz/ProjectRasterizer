#include "ShaderD3D11.h"

ShaderD3D11::~ShaderD3D11()
{
}

ShaderD3D11::ShaderD3D11(ID3D11Device* device, ShaderType shaderType, const void* dataPtr, size_t dataSize) : type(shaderType)
{
	Initialize(device, shaderType, dataPtr, dataSize);
}

ShaderD3D11::ShaderD3D11(ID3D11Device* device, ShaderType shaderType, const char* csoPath) : type(shaderType)
{
	Initialize(device, shaderType, csoPath);
}

void ShaderD3D11::Initialize(ID3D11Device* device, ShaderType shaderType, const void* dataPtr, size_t dataSize)
{
	switch (this->type) {
	case ShaderType::VERTEX_SHADER:
		if (FAILED(device->CreateVertexShader(dataPtr, dataSize, nullptr, &shader.vertex)))
			throw std::runtime_error("Failed to create vertex shader");
		break;
	case ShaderType::HULL_SHADER:
		if (FAILED(device->CreateHullShader(dataPtr, dataSize, nullptr, &shader.hull)))
			throw std::runtime_error("Failed to create hull shader");
		break;
	case ShaderType::DOMAIN_SHADER:
		if (FAILED(device->CreateDomainShader(dataPtr, dataSize, nullptr, &shader.domain)))
		{
			throw std::runtime_error("Failed to create domain shader");
		}
		break;
	case ShaderType::GEOMETRY_SHADER:
		if (FAILED(device->CreateGeometryShader(dataPtr, dataSize, nullptr, &shader.geometry)))
		{
			throw std::runtime_error("Failed to create geometry shader");
		}
		break;
	case ShaderType::PIXEL_SHADER:
		if (FAILED(device->CreatePixelShader(dataPtr, dataSize, nullptr, &shader.pixel)))
		{
			throw std::runtime_error("Failed to create pixel shader");
		}
		break;
	case ShaderType::COMPUTE_SHADER:
		if (FAILED(device->CreateComputeShader(dataPtr, dataSize, nullptr, &shader.compute)))
		{
			throw std::runtime_error("Failed to create compute shader");
		}
		break;
	default:
		break;
	}
}

void ShaderD3D11::Initialize(ID3D11Device* device, ShaderType shaderType, const char* csoPath)
{
	std::ifstream reader;
	reader.open(csoPath, std::ios::binary | std::ios::ate);

	if (!reader.is_open())
	{
		std::cerr << "Could not open VS file!" << std::endl;
		throw std::runtime_error("Could not open VS file!");
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	Initialize(device, shaderType, shaderData.data(), shaderData.size());

	reader.close();
}

const void* ShaderD3D11::GetShaderByteData() const
{
	return shaderData.data();
}

size_t ShaderD3D11::GetShaderByteSize() const
{
	return this->shaderData.size();
}

void ShaderD3D11::BindShader(ID3D11DeviceContext* context) const
{
	switch (this->type) {
	case ShaderType::VERTEX_SHADER:
		context->VSSetShader(this->shader.vertex, nullptr, 0);
		break;
	case ShaderType::HULL_SHADER:
		context->HSSetShader(this->shader.hull, nullptr, 0);
		break;
	case ShaderType::DOMAIN_SHADER:
		context->DSSetShader(this->shader.domain, nullptr, 0);
		break;
	case ShaderType::GEOMETRY_SHADER:
		context->GSSetShader(this->shader.geometry, nullptr, 0);
		break;
	case ShaderType::PIXEL_SHADER:
		context->PSSetShader(this->shader.pixel, nullptr, 0);
		break;
	case ShaderType::COMPUTE_SHADER:
		context->CSSetShader(this->shader.compute, nullptr, 0);
		break;
	default:
		break;
	}
}
