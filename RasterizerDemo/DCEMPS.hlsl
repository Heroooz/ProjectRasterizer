sampler samplerState : register(s0);

Texture2D ambientTexture : register(t0);
Texture2D diffuseTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D normalTexture : register(t3);
TextureCube reflectionTexture : register(t4);

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

struct DCEMPSOutput
{
    float4 position : SV_Target0;   // Position (XYZ) + Ambient Factor (W)
    float4 normal : SV_Target1;     // Normal (XYZ) + Specualr Factor (W)
    float4 diffuse : SV_Target2;    // Diffuse (XYZ) + Shiniess (W)
};

struct DCEMPSInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

static const float defAmb = 1.0;
DCEMPSOutput main(DCEMPSInput input)
{
    DCEMPSOutput output;
    
    float3 normal = normalize(input.normal);
    
    float3 viewDir = normalize(cameraPosition.xyz - input.worldPosition.xyz); // View direction from the camera to the fragment
    float3 samplevec = normalize(reflect(-viewDir, normalize(input.normal.xyz))); // Reflect the view direction around the normal to get the reflection vector
    float3 reflection = reflectionTexture.Sample(samplerState, samplevec).rgb; // Sample the texture using the vector to get the color
    
    float ambient = (reflection.r + reflection.g + reflection.b) / 3.0f;
    float specular = (specularFactor.x + specularFactor.y + specularFactor.z) / 3;
    float4 diffuse = float4(diffuseFactor, 1.0f) * float4(reflection, 1.0f);
    
    output.position = float4(input.worldPosition.xyz, ambient);
    output.normal = float4(normal, specular);
    output.diffuse = float4(diffuse.xyz, shininess);
    
    return output;
};
