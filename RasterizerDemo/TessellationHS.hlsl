cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
    float3 cameraPosition;
    float padding;
};


cbuffer Object : register(b1)
{
    float4 centerPoint;
}

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3


static float maxDist = 20.0f;
static float minDist = 1.0f;
static float base = 15.0f;

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
    InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip)
{
    HS_CONSTANT_DATA_OUTPUT output;
    float distance = length(cameraPosition - centerPoint.xyz);
    output.EdgeTessFactor[0] = output.EdgeTessFactor[1] = output.EdgeTessFactor[2] = output.InsideTessFactor = max(minDist, maxDist - distance);
    
    return output;
};

struct HullShaderOutput
{
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};


[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]

HullShaderOutput main(
 InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip,
 uint i : SV_OutputControlPointID)
{
    HullShaderOutput output;
    
    output.worldPosition = ip[i].worldPosition;
    output.normal = ip[i].normal;
    output.uv = ip[i].uv;

    return output;
};