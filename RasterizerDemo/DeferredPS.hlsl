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
    float shininess;
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


struct PSOutPut
{
    float4 position : SV_Target0;       // Position + Ambient Factor
    float4 normal   : SV_Target1;       // Normal   + Specualr Factor
    float4 diffuse    : SV_Target2;     // Diffuse
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : UV;
};

static const float defAmb = 0.1;
static const float layerDepth = 1.0f / 16.9f;
PSOutPut main(PSInput input)
{
    // For the objs, UV-y must be flipped :)
    float2 uv;
    uv.x = input.uv.x;
    uv.y = -input.uv.y;
    
    float4 position = input.worldPosition;
    float3 normal = normalize(input.normal.xyz);
    
    float ambient = ambientFactor;
    if (hasAmbientTexture)
    {
        ambient *= ambientTexture.Sample(samplerState, uv).x;
    }
    float specular = specularFactor;
    if (hasSpecularTexture)
    {
        specular *= specularTexture.Sample(samplerState, uv);
    }
    
    float4 diffuse = float4(diffuseTexture.Sample(samplerState, uv));;
    
    float2 samplePoint = uv;;
    
    // Calculating normal map and parallaxing
    if (hasNormalTexture)
    {
        // Calculating the tangent and bitangent of the vertex
        float3 tangent;
        float3 bitangent;
        ComputeTBN(input.worldPosition.xyz, normal, uv, tangent, bitangent);
        
        float3x3 tbn = float3x3(tangent, bitangent, normal);
        
        // View Direction in tangent space
        float3 viewDir = normalize(mul(cameraPosition - input.worldPosition.xyz, transpose(tbn)));
        
        // Parallaxing
        float2 offset = float2(0, 0);
        float2 prev = offset;
        float current = 0.0f;
        
        float2 delta = -viewDir.xy * (parallax / viewDir.z) * layerDepth;
        
        float heightSample;
        for (int i = 0; i < 8; i++)
        {
            prev = offset;
            current += layerDepth;
            offset += delta;
            
            heightSample = normalTexture.Sample(samplerState, uv + offset).w;
            
            // If gone too far, back up
            if (current >= heightSample)
            {
                float prevD = current - layerDepth;
                float t = (heightSample - prevD) / layerDepth;
                offset = lerp(prev, offset, t);
                break;
            }
        }
        samplePoint += offset;
                
        float3 normalMap = normalTexture.Sample(samplerState, samplePoint).rgb * 2.0f - 1.0f;
        float3 worldNormal = normalize(normalMap.x * tangent + normalMap.y * bitangent + normalMap.z * input.normal);
        
        normal = float4(worldNormal, 0);            // Saving normal W for specular
    }
    else normal = normalize(float4(input.normal, 0));
    
    
    PSOutPut output;
    output.position = float4(input.worldPosition.xyz, ambient);         // Position XYZ + Ambient W
    output.normal = float4(normal, specular);                           // Normal XYZ + Specular W
    output.diffuse = float4(diffuseTexture.Sample(samplerState, uv));   // Diffuse
    
    return output;
    
    //float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition.xyz);
    //float3 viewDirection = normalize(cameraPosition.xyz - input.worldPosition.xyz);
    //float3 halfVector = normalize(lightDirection + viewDirection);
    //float4 ambient = float4(ambientFactor, 0) * lightColor * intensity;
    //float4 diffuse = float4(diffuseFactor, 0) * lightColor * saturate(dot(normal, lightDirection));
    //float4 specular = float4(specularFactor, 0) * lightColor * pow(saturate(dot(normal, halfVector)), shininess);
    
    //float4 ambient = float4(ambientFactor * defAmb, 1) * diffuseTexture.Sample(samplerState, input.uv);
    //float4 diffuse = float4(diffuseFactor, 1) * diffuseTexture.Sample(samplerState, input.uv);
    //float4 specular = float4(specularFactor, 1);// * pow(saturate(dot(normal, halfVector)), shininess);
};

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
        float3 up = abs(normal.z < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0));
        tangent = normalize(cross(up, normal));
        bitangent = normalize(cross(normal, tangent));
    }
};