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

void ComputeTBN(in float3 worldPos, in float3 normal, in float2 uv, out float3 tangent, out float3 bitangent);

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
    // For the objs, UV-y must be flipped :)
    float2 uv = input.uv;
    uv.y = -uv.y;
 
    // Standard Normal
    float3 normal = normalize(input.normal);
    
    float3 ambient = defAmb * ambientFactor;
    if (hasAmbientTexture == 1)
    {
        ambient *= ambientTexture.Sample(samplerState, uv).rgb;
    }
    float3 specular = specularFactor;
    if (hasSpecularTexture == 1)
    {
        specular *= specularTexture.Sample(samplerState, uv).rgb;
    }
    
    float4 diffuse = float4(diffuseFactor, 1);
    if (hasDiffuseTexture == 1)
        diffuse *= float4(diffuseTexture.Sample(samplerState, uv));;
    
    
    // Calculating normal map and parallaxing
    //float2 sampelingUV = uv;    // UV for parallaxing
    if (hasNormalTexture == 1)
    {
        // Calculating the tangent and bitangent of the vertex
        float3 tangent;
        float3 bitangent;
        ComputeTBN(input.worldPosition.xyz, normal, input.uv, tangent, bitangent);
        
        float3x3 tbn = float3x3(tangent, bitangent, normal);
        
        // View Direction in tangent space
        float3 viewDir = normalize(mul(cameraPosition - input.worldPosition.xyz, transpose(tbn)));
        
        // Parallaxing
        float2 offset = float2(0, 0);
        float2 prev = offset;
        float current = 0.0f;
        
        float2 delta = -viewDir.xy * (parallax / viewDir.z) * layerDepth;
        
        float heightSample;
        for (int i = 0; i < nrOfLayers; i++)
        {
            prev = offset;
            current += layerDepth;
            offset += delta;
            
            heightSample = normalTexture.Sample(samplerState, uv + offset).a;
            
            
            
            // If gone too far, back up
            //if (current >= heightSample)
            //{
            //    float prevD = current - layerDepth;
            //    float t = (heightSample - prevD) / layerDepth;
            //    offset = lerp(prev, offset, t);
            //    break;
            //}
        }
        float2 sampelingUV = input.uv + offset;
                
        float3 normalMap = normalTexture.Sample(samplerState, sampelingUV).rgb * 2.0f - 1.0f;
        
        float3 worldNormal = normalize(normalMap.x * tangent + normalMap.y * bitangent + normalMap.z * normal);
        
        normal = worldNormal; // Saving normal W for specular
    }
    
    //return float4(saturate(diffuse.rgb * ambient + diffuse.rgb + specular), 1);
    return diffuse;
};

// Works because only simple meshes are allowed to have normal maps :)
void ComputeTBN(in float3 worldPos, in float3 normal, in float2 uv, out float3 tangent, out float3 bitangent)
{
    float3 dpdx = ddx(worldPos);
    float3 dpdy = ddy(worldPos);
    float2 duvdx = ddx(uv);
    float2 duvdy = ddy(uv);

    float det = duvdx.x * duvdy.y - duvdx.y * duvdy.x;
    if (abs(det) > 1e-6)
    {
        tangent = normalize((duvdy.y * dpdx - duvdx.y * dpdy) / det);
        bitangent = normalize((-duvdy.x * dpdx + duvdx.x * dpdy) / det);

        tangent = normalize(tangent - normal * dot(normal, tangent));
        bitangent = normalize(bitangent - normal * dot(normal, bitangent));
    }
    else
    {
        float3 up = (abs(normal.z) < 0.999) ? float3(0, 0, 1) : float3(1, 0, 0);
        tangent = normalize(cross(up, normal));
        bitangent = normalize(cross(normal, tangent));
    }
};
