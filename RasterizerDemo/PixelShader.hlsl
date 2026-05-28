Texture2D shaderTexture : register(t0);
SamplerState samplerState : register(s0);

//Texture2D ambientTexture    : register(t0);
//Texture2D diffuseTexture    : register(t1);
//Texture2D specularTexture   : register(t2);
//Texture2D bumpTexture       : register(t3);
//Texture2D normalTexture     : register(t4);

//cbuffer CameraBuffer : register(b0)
//{
//    float4x4 viewProjMatrix;
//    float3 cameraPosition;
//    float padding;
//};

//cbuffer LightBuffer : register(b1)
//{
//    float3 lightPosition;
//    float Intensity;
//    float3 lightColor;
//    float padding;
//};

//cbuffer MaterialBuffer : register(b2)
//{
//    float3 ambient;
//    float3 diffuse;
//    float3 specular;
//    float shininess;
	
//    bool ambientTexture;
//    bool diffuseTexture;
//    bool specularTexture;
//    bool bumpTexture;
//    bool normalTexture;
//};


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
	float3 viewDirection = normalize(cameraPosition.xyz - input.worldPosition.xyz);
	float3 halfVector = normalize(lightDirection + viewDirection);

	float4 ambient = lightColor * intensity * 2;
	float4 diffuse = lightColor * saturate(dot(normal, lightDirection));
	float4 specular = lightColor * pow(saturate(dot(normal, halfVector)), shininess);

	return (ambient + diffuse) * shaderTexture.Sample(samplerState, input.uv) + specular;

}