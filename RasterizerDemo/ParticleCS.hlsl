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

[numthreads(32,1,1)]
void main( uint3 DTid : SV_DispatchThreadID)
{
    Particle gettingProcessed = Particles[DTid.x];
    
    // Logic ,amipulating "gettingProessed" goes here
    
    gettingProcessed.position += gettingProcessed.speed * gettingProcessed.dir;
    
    if (length(gettingProcessed.position - gettingProcessed.startPos) > gettingProcessed.dist)
        gettingProcessed.position = gettingProcessed.startPos;
    
    
    Particles[DTid.x] = gettingProcessed;
}
