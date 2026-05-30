RWTexture2D<unorm float4> backBufferUAV : register(u0);

SamplerState samplerState : register(s0);

Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> normalGBuffer : register(t1);
Texture2D<float4> diffuseGBuffer : register(t2);

struct LightBuffer
{
    float4x4 vpmatrix;
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float angle;
};

StructuredBuffer<LightBuffer> SpotLights : register(t3);
StructuredBuffer<LightBuffer> DirLights : register(t4);


cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
    float3 cameraPosition;
    float padding;
};


cbuffer NrOfLights : register(b2)
{
    int nrofSpotLights;
    int nrofDirLights;
};


[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixelPosition = float4(positionGBuffer[DTid.xy].xyz, 1);     // Position
    float4 pixelNormal = float4(normalGBuffer[DTid.xy].xyz, 1);         // Normal
    
    float4 diffuse = 0;
    float4 specular = 0;
    float specularKof = 100;
    
    
    // Light Calculations for SpotLights
    for (int i = 0; i < nrofSpotLights; i++)
    {
        LightBuffer spotlight = SpotLights[i];
        
        float3 L = normalize(spotlight.position - pixelPosition.xyz);   // LightDir
        float3 V = normalize(cameraPosition.xyz - pixelPosition.xyz);   // ViewDir
        float h = L + V;
        float3 H = normalize(h / dot(h, h));                            // Half-Vector
        float S = dot(pixelNormal.xyz, H);                              // Sigma between Halfway-Vector & Normal
       
        float spec = max(dot(pixelNormal.xyz, H), 0);                   // Specular Intensity
        
        // Setting the diffuse and specular    
        diffuse += float4(diffuseGBuffer[DTid.xy].xyz, 0) * spotlight.intensity * spotlight.color;
        specular += spotlight.color * spec * specularKof * spotlight.intensity;
    }
    
    // Light Calculations for Directional Lights
    for (int i = 0; i < nrofDirLights; i++ )
    {
        LightBuffer dirlight = DirLights[i];
        
        float3 L = normalize(dirlight.direction.xyz);
        float3 V = normalize(cameraPosition.xyz - pixelPosition.xyz);
        float3 h = L + V;
        float3 H = normalize(h / dot(h, h));
        float S = dot(pixelNormal.xyz, H);
        
        
        diffuse += float4(diffuseGBuffer[DTid.xy].xyz, 0) * dirlight.intensity * dirlight.color;
        specular += dirlight.intensity * pow(saturate(S), specularKof) * dirlight.color;
    }
    
    float4 ambient = positionGBuffer[DTid.xy].a * float4(diffuseGBuffer[DTid.xy].xyz, 0);
    backBufferUAV[DTid.xy] = ambient + diffuse + specular;
    
      
  
    return;
}