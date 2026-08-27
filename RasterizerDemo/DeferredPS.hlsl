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
    float shininess;        // Phong-Exponent
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
    float4 position : SV_Target0;       // Position (XYZ) + Ambient Factor (W)
    float4 normal   : SV_Target1;       // Normal (XYZ) + Specualr Factor (W)
    float4 diffuse  : SV_Target2;       // Diffuse (XYZ)
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

static const float defAmb = 0.2f;
static const int nrOfLayers = 32;
static const float layerDepth = 1.0f / nrOfLayers;
PSOutPut main(PSInput input)
{
    // For the objs, UV-y must be flipped :)
    float2 uv = input.uv;
    uv.y = -uv.y;
 
    // Standard Normal
    float3 normal = normalize(input.normal);
    float ambient = defAmb * (ambientFactor.x + ambientFactor.y + ambientFactor. z) / 3;
    float specular = (specularFactor.x + specularFactor.y + specularFactor.z) / 3 * shininess;
    float4 diffuse = float4(diffuseFactor, 1);

    // Calculating normal map and parallaxing
    if (hasNormalTexture == 1)
    {
        // Calculating the tangent and bitangent of the vertex
        float3 tangent;
        float3 bitangent;
        ComputeTBN(input.worldPosition.xyz, normal, input.uv, tangent, bitangent);
        float3x3 tbn = float3x3(tangent, bitangent, normal);
        
        // View Direction in tangent space
        float3 viewDir = normalize(mul(cameraPosition - input.worldPosition.xyz, transpose(tbn)));      
           
        float2 texStep = -(viewDir.xy / viewDir.z) * (parallax * layerDepth);
        float currentLayerDepth = 0.0f;
        
        float2 currentUV = input.uv;
        float2 prevUV = input.uv;
        float beforeSampleDepth = 0.0f;
        float afterSampleDepth = 0.0f;
        for (int i = 0; i < nrOfLayers; i++)
        {
            prevUV = currentUV;
            currentUV += texStep;
            currentLayerDepth += layerDepth;
            
            beforeSampleDepth = afterSampleDepth;
            afterSampleDepth = normalTexture.Sample(samplerState, currentUV).a;
            
            // Should not go under the face
            if (currentLayerDepth >= afterSampleDepth)
            {
                float beforeDiff = beforeSampleDepth - (currentLayerDepth - layerDepth);
                float afterDiff = currentLayerDepth - afterSampleDepth;
                float weight = afterDiff / (beforeDiff + afterDiff);
                
                uv = lerp(currentUV, prevUV, weight);
                break;
            }
        }
        
        float3 normalMap = normalTexture.Sample(samplerState, uv).rgb * 2.0f - 1.0f;
        float3 worldNormal = normalize(normalMap.x * tangent + normalMap.y * bitangent + normalMap.z * normal);
        normal = worldNormal;
    }
    
    if (hasAmbientTexture == 1)
        ambient *= ambientTexture.Sample(samplerState, uv).r;
    if (hasDiffuseTexture == 1)
        diffuse *= float4(diffuseTexture.Sample(samplerState, uv));
    if (hasSpecularTexture == 1)
        specular *= specularTexture.Sample(samplerState, uv).r;
    
    
    PSOutPut output;
    output.position = float4(input.worldPosition.xyz, ambient); // Position XYZ + Ambient W
    output.normal = float4(normal, specular); // Normal XYZ + Specular W
    output.diffuse = diffuse; 
    return output;
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

        /* INTERPOLATION MATH
        float beforeDiff = beforePointDepth - beforeSampleDepth;
        float afterDiff = afterSampleDepth - afterPointDepth;
        float totalDiff = before + afterDiff;
        float weight = 1.0f - (beforeDiff / totalDiff);
        return beforeCoords * weight + afterCoords * (1.0f - weight);
        */

  
        /*
        // ~p approximated point
        // h(p) depth value at pont p (from disp. map)
        // v viewdir (in tangenspace!!)
        // ~p = - (h(p) * v_xy) / v_z
        */
