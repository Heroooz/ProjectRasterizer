#pragma once
#include <d3d11_4.h>
#include <vector>
#include "ParticlesBuffer.h"


struct ParticleData
{
	float position[3];
	float speed;
	float startPos[3];
	float dist;
	float dir[3];
	float padding;
	;
};

class Particles
{
	ParticleBuffer particles;
	UINT nrofParticles = 0;


public:
	Particles(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true,
		bool hasSRV = false, bool hasUAV = false);
	~Particles();

	void Initialize(ID3D11Device* device, UINT sizeOfElement,
		size_t nrOfElementsInBuffer, void* bufferData = nullptr, bool dynamic = true,
		bool hasSRV = false, bool hasUAV = false);

	//void Update(ID3D11DeviceContext* immediateContext, float deltatime);

};
