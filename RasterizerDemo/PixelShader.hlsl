Texture2D shaderTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer ConstBuffer : register(b0)
{
    float4 lightPosition;
    float4 lightColor;
    float4 cameraPosition;
    float intensity;
    float shininess;
    float padding[2]; 
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 worldPosition : WORLD_POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition.xyz);
    
    float4 ambient = lightColor * intensity;
    float4 diffuse = lightColor * saturate(dot(normal, lightDirection));
    float4 specular = lightColor * pow(saturate(dot(reflect(-lightDirection, normal), normalize(cameraPosition.xyz - input.worldPosition.xyz))), shininess);
    
    return (ambient + diffuse) * shaderTexture.Sample(samplerState, input.uv) + specular;
}