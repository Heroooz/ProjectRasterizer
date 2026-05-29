struct PSOutPut
{
    float4 position : SV_Target0;
    float4 color    : SV_Target1;
    float4 normal   : SV_Target2;
};


struct PSInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

PSOutPut main(PSInput input)
{
    PSOutPut output;
    
    output.position = input.worldPosition;
    output.color = float4(1.0, 1.0, 1.0, 1.0);
    output.normal = float4(normalize(input.normal), 0.0);
    
    return output;
};