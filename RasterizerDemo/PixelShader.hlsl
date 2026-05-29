//Texture2D shaderTexture : register(t0);
SamplerState samplerState : register(s0);

Texture2D ambientTexture : register(t0);
Texture2D diffuseTexture : register(t1);
Texture2D specularTexture : register(t2);
//Texture2D bumpTexture       : register(t3);
//Texture2D normalTexture     : register(t4);

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
    float3 cameraPosition;
    float padding;
};

cbuffer LightBuffer : register(b1)
{
    float4 lightColor;
    float3 lightPosition;
    float intensity;
    //float padding1;
};

cbuffer MaterialBuffer : register(b2)
{
    float3 ambientFactor;
    float shininess;
    float3 diffuseFactor;
    float padding2;
    float3 specularFactor;
    float padding3;
	
    //bool hasAmbientTexture;
    //bool hasDiffuseTexture;
    //bool hasSpecularTexture;
    //bool hasBumpTexture;
    //bool hasNormalTexture;
};


//cbuffer ConstBuffer : register(b0)
//{
//	float4 lightPosition;
//	float4 lightColor;
//	float4 cameraPosition;
//	float intensity;
//	float shininess;
//	float padding[2]; 
//};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 worldPosition : WORLD_POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    // For the objs, UV-y must be flipped :)
    input.uv[1] = -input.uv[1];
	float3 normal = normalize(input.normal);
	float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition.xyz);
	float3 viewDirection = normalize(cameraPosition.xyz - input.worldPosition.xyz);
	float3 halfVector = normalize(lightDirection + viewDirection);

	float4 ambient = float4(ambientFactor, 0) * lightColor * intensity;
    float4 diffuse = float4(diffuseFactor, 0) * lightColor * saturate(dot(normal, lightDirection));
    float4 specular = float4(specularFactor, 0) * lightColor * pow(saturate(dot(normal, halfVector)), shininess);

	return (ambient + diffuse) * diffuseTexture.Sample(samplerState, input.uv) + specular;

}