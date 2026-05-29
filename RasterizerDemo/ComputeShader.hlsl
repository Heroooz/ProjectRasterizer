RWTexture2D<unorm float4> backBufferUAV : register(u0);

Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> colorGBuffer : register(t1);
Texture2D<float4> normalGBuffer : register(t2);


[numthreads(8, 8, 1)]

void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float3 position = positionGBuffer[DTid.xy].xyz;
    float3 colour = positionGBuffer[DTid.xy].xyz;
    float3 normal = normalize(normalGBuffer[DTid.xy].xyz);
    //Perform lighting
    //calculations...
    
    float3 result = colour;
    
    backBufferUAV[DTid.xy] = float4(result, 1.0f);
    return;
}