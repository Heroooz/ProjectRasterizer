SamplerState samplerState : register(s0);

Texture2D ambientTexture : register(t0);
Texture2D diffuseTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D normalTexture : register(t3);

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
    float3 cameraPosition;
    float padding;
};

cbuffer MaterialBuffer : register(b1)
{
    float3 ambientFactor;
    float shininess; // Phong-Exponent
    float3 diffuseFactor;
    float parallax;
    float3 specularFactor;
    float padding3;
    int hasAmbientTexture;
    int hasDiffuseTexture;
    int hasSpecularTexture;
    int hasNormalTexture;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

static const float defAmb = 0.1f;
static const int nrOfLayers = 128;
static const float layerDepth = 1.0f / nrOfLayers;

float4 main(PSInput input) : SV_Target
{
    // For the objs, UV-y must be flipped, because DirectX and OpenGL have different UV coordinate systems.
    float2 uv = input.uv;
    uv.y = -uv.y;
 
    float3 normal = normalize(input.normal);
    
    float3 ambient = defAmb * ambientFactor;
    if (hasAmbientTexture == 1)
        ambient *= ambientTexture.Sample(samplerState, uv).rgb;
    
    float4 diffuse = float4(diffuseFactor, 1);
    if (hasDiffuseTexture == 1)
        diffuse *= float4(diffuseTexture.Sample(samplerState, uv));

    float3 specular = specularFactor;
    if (hasSpecularTexture == 1)
        specular *= specularTexture.Sample(samplerState, uv).rgb;
   
    return float4(saturate((ambient + diffuse.rgb + specular)), 1);
}
