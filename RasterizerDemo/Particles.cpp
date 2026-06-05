#include "Particles.h"

Particles::Particles(ID3D11Device* device, UINT sizeOfElement,
	size_t nrOfElementsInBuffer, void* bufferData, bool dynamic,
	bool hasSRV, bool hasUAV)
{
	Initialize(device, sizeOfElement, nrOfElementsInBuffer, bufferData, dynamic, hasSRV, hasUAV);
}
Particles::~Particles()
{

}

void Particles::Initialize(ID3D11Device* device, UINT sizeOfElement,
	size_t nrOfElementsInBuffer, void* bufferData, bool dynamic,
	bool hasSRV, bool hasUAV)
{
	std::vector<ParticleData> particles;
	this->nrofParticles = nrOfElementsInBuffer;

	for (size_t i = 0; i < nrofParticles; i++)
	{
		int randNum = (rand() % 20) / 10;
		particles[i].position[0] = randNum;
		particles[i].position[1] = 0;
		particles[i].position[2] = randNum;

		particles[i].speed = 0;

		//particles[i].lifesp = 0;
	}

	this->particles.Initialize(device, sizeOfElement, nrOfElementsInBuffer, particles.data(), dynamic, hasSRV, hasUAV);
}

//void Particles::Update()
//{
//
//}
