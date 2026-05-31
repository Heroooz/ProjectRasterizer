SamplerState samplerState : register(s0);

Texture2D ambientTexture : register(t0);
Texture2D diffuseTexture : register(t1);
Texture2D specularTexture : register(t2);
Texture2D normalTexture : register(t3);
//Texture2D bumpTexture       : register(t3);

cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
    float3 cameraPosition;
    float padding;
};

//cbuffer LightBuffer : register(b1)
//{
//    float4 lightColor;
//    float3 lightPosition;
//    float intensity;
//    //float padding1;
//};

cbuffer MaterialBuffer : register(b1)
{
    float3 ambientFactor;
    float shininess;
    float3 diffuseFactor;
    float padding2;
    float3 specularFactor;
    float padding3;
	
    int hasAmbientTexture;
    int hasDiffuseTexture;
    int hasSpecularTexture;
    int hasNormalTexture;
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
 //   input.uv[1] = -input.uv[1];
	//float3 normal = normalize(input.normal);
	////float3 lightDirection = normalize(lightPosition.xyz - input.worldPosition.xyz);
	////float3 viewDirection = normalize(cameraPosition.xyz - input.worldPosition.xyz);
	////float3 halfVector = normalize(lightDirection + viewDirection);

	//float4 ambient = float4(ambientFactor, 0);// * lightColor * intensity ;
 //   float4 diffuse = float4(diffuseFactor, 0);// * lightColor * saturate(dot(normal, lightDirection));
 //   float4 specular = float4(specularFactor, 0);// * lightColor * pow(saturate(dot(normal, halfVector)), shininess);

	//return (ambient + diffuse) * diffuseTexture.Sample(samplerState, input.uv) * 0.2 + specular;

    float2 uv = input.uv;
    uv.y = -uv.y;
    
    float4 diffuse = diffuseTexture.Sample(samplerState, uv) * float4(diffuseFactor, 1); // * lightColor * saturate(dot(normal, lightDirection));
    float ambient = 0.2 * ambientFactor;// * diffuseTexture.Sample(samplerState, uv).xyz;
    if (hasAmbientTexture == 1)
    {
        ambient *= ambientTexture.Sample(samplerState, uv).x;
    }
    float specular = specularFactor * shininess * diffuseTexture.Sample(samplerState, uv).xyz;
    if (hasSpecularTexture == 1)
    {
        specular *= specularTexture.Sample(samplerState, uv);
    }
    
    return diffuse * ambient;

    //uv[1] = -input.uv[1];
    
    float4 position = input.worldPosition;
    float3 normal = input.normal;
    
    
    
    //PSOutPut output;
    //output.position = float4(input.worldPosition.xyz, ambient); // Position XYZ + Ambient W
    //output.normal = float4(normal, specular); // Normal XYZ + Specular W
    //output.diffuse = diffuse; // Diffuse
    
   
    
}