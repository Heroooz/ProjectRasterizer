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

	unsigned char* textureData = stbi_load("Particles/blackSmoke16.png", &width, &height, &channels, 4);
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
		ParticleData particledata = {};

		float px = float(rand() % 20) * 0.1f + 12.0f;	// startpos.x
		float pz = float(rand() % 20) * 0.1f + 4.5f;	// startpos.z
		float ax = float(rand() % 40000 - 20000) * 0.00001f;		// dir.x
		float ay = float(rand() % 40000 - 20000) * 0.00001f;		// dir.z
		float speed = float(rand() % 50) * 0.001f;		// speed
		float dist = float(rand() % 50) + 3.0f;			// distance to travel


		particledata.position = particledata.startPos = { px, 7.0f, pz };
		particledata.dir = { ax, 1.0f, ay };
		particledata.speed = speed;
		particledata.dist = dist;

		particles.push_back(particledata);
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
