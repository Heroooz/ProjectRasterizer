Texture2D particleTexture : register(t0);
SamplerState particleSampler : register(s0);

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
};

struct PixelShaderOutput
{
    float4 position : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 color : SV_TARGET2;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput psout;
    
    psout.position = input.pos;
    psout.normal = input.normal;
    float4 color = particleTexture.Sample(particleSampler, input.uv);
    
    if (color.a <= 0.05f)
    {
        discard; // Discard the pixel if transparency is below threshold
    };
    psout.color = color;
    
    return psout;
}