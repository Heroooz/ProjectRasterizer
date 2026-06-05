#pragma once
#include <d3d11_4.h>
#include <vector>
#include "stb_image.h"
#include "ParticlesBuffer.h"


struct ParticleData
{
	XMFLOAT3 position;
	float speed;
	XMFLOAT3 startPos;
	float dist;
	XMFLOAT3 dir;
	float padding = 0.0f;
};

class Particles
{
	ParticleBuffer particlesBuffer;
	int nrofParticles = 0;

	ID3D11ShaderResourceView* particleTexture;


public:
	Particles(ID3D11Device* device, UINT sizeOfElement,
		UINT nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true,
		bool hasSRV = false, bool hasUAV = false);
	~Particles();

	void Initialize(ID3D11Device* device, UINT sizeOfElement,
		UINT nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true,
		bool hasSRV = false, bool hasUAV = false);

	int GetNrOfParticles();
	ID3D11ShaderResourceView* GetSRV();
	ID3D11ShaderResourceView* GetTexture();
	ID3D11UnorderedAccessView* GetUAV();
	ID3D11Buffer* GetParticlesBuffer();
	//void Update(ID3D11DeviceContext* immediateContext, float deltatime);
};
