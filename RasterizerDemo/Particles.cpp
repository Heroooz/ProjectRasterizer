#include "Particles.h"

Particles::Particles(ID3D11Device* device, UINT sizeOfElement,
	UINT nrOfElementsInBuffer, void* bufferData, bool dynamic,
	bool hasSRV, bool hasUAV)
{
	Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic, hasSRV, hasUAV);
}
Particles::~Particles()
{
	if (this->particleTexture)
	{
		this->particleTexture->Release();
		this->particleTexture = nullptr;
	}
}

void Particles::Initialize(ID3D11Device* device, UINT sizeOfElement,
	UINT nrOfElementsInBuffer, void* bufferData, bool dynamic,
	bool hasSRV, bool hasUAV)
{
	int width, height, channels;

	unsigned char* textureData = stbi_load("explosion00.png", &width, &height, &channels, 4);
	if (textureData == nullptr)
		throw std::runtime_error("Failed to load texture!");

	ID3D11Texture2D* texture;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = textureData;
	data.SysMemPitch = width * 4;
	data.SysMemSlicePitch = 0;

	if (FAILED(device->CreateTexture2D(&textureDesc, &data, &texture)))
		throw std::runtime_error("Failed to create texture!");

	if (FAILED(device->CreateShaderResourceView(texture, nullptr, &particleTexture)))
		throw std::runtime_error("Failed to create texture reasource view!");


	this->nrofParticles = nrOfElementsInBuffer;

	std::vector<ParticleData> particles;
	for (size_t i = 0; i < nrofParticles; i++)
	{
		ParticleData data = {};
		//float dist = (float)(rand() % 20) / 10 - 2;
		//float startpos = (float)(rand() % 20 / 10);

		//data.dir = XMFLOAT3(0.0f, 0.0f, 0.0f);
		//data.position = data.startPos = { startpos,startpos,startpos };
		//data.dist = dist;
		//data.speed = dist;

		data.position = data.startPos = { 5.0f,2.0f,3.0f };
		data.dir = { 0.0f,0.0f,0.0f };
		data.speed = 0.0f;
		data.dist = 1.0f;


		particles.push_back(data);
	}

	this->particlesBuffer.Initialize(device, sizeOfElement, nrOfElementsInBuffer, particles.data(), dynamic, hasSRV, hasUAV);

	texture->Release();
	stbi_image_free(textureData);
}


int Particles::GetNrOfParticles() { return this->nrofParticles; }

ID3D11ShaderResourceView* Particles::GetSRV() { return this->particlesBuffer.GetSRV(); }
ID3D11ShaderResourceView* Particles::GetTexture() { return this->particleTexture; } 
ID3D11UnorderedAccessView* Particles::GetUAV() { return this->particlesBuffer.GetUAV(); }
ID3D11Buffer* Particles::GetParticlesBuffer() { return this->particlesBuffer.GetBuffer(); }

//void Particles::Update()
//{
//
//}
