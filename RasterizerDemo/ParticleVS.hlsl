struct Particle
{
    float3 position;
    float speed;
    float3 startPos;
    float dist;
    float3 dir;
    float padding;
};

StructuredBuffer<Particle> Particles : register(t0);

float3 main(uint vertexID : SV_VertexID) : POSITION
{
    return Particles[vertexID].position;
}