struct Particle
{
    float3 position;
    float speed;
    float3 startPos;
    float dist;
    float3 dir;
    float padding;
};

RWStructuredBuffer<Particle> Particles : register(u0);

[numthreads(32, 1, 1)] // Devide particles into groups of 32 
void main( uint3 DTid : SV_DispatchThreadID)
{
    Particle gettingProcessed = Particles[DTid.x]; // Get the particle at the current thread ID
    
    gettingProcessed.position += gettingProcessed.speed * gettingProcessed.dir;
    
    // If the particle has moved beyond its distance "remove" and reuse as "new" particle
    if (length(gettingProcessed.position - gettingProcessed.startPos) > gettingProcessed.dist)
        gettingProcessed.position = gettingProcessed.startPos;
    
    Particles[DTid.x] = gettingProcessed;
}
