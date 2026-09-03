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
static float base = 10.0f;

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
    InputPatch<VertexShaderOutput, NUM_CONTROL_POINTS> ip)
{
    HS_CONSTANT_DATA_OUTPUT output;
    float distance = length(cameraPosition - centerPoint.xyz);
    output.EdgeTessFactor[0] = output.EdgeTessFactor[1] = output.EdgeTessFactor[2] = output.InsideTessFactor = lerp(base, minDist, saturate(distance/maxDist));
    
    return output;
};

struct HullShaderOutput
{
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};


[domain("tri")]
[partitioning("fractional_odd")] // Fractional odd tessellation partitioning
[outputtopology("triangle_cw")] // Clockwise winding order for the output triangles
[outputcontrolpoints(3)] 
[patchconstantfunc("CalcHSPatchConstants")] // Function to calculate tessellation factors for the patch

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