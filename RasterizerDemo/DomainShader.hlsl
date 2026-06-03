cbuffer CameraBuffer : register(b0)
{
    float4x4 vp;
    float3 cameraPosition;
    float padding;
};

struct DomainShaderOutput
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HullShaderOutput
{
    float3 worldPosition : WORLD_POSITION;
    float3 normal : NORMAL;
    float2 uv : UV;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3
[domain("tri")]
DomainShaderOutput main(HS_CONSTANT_DATA_OUTPUT input, float3 barycentric : SV_DomainLocation,
    const OutputPatch<HullShaderOutput, NUM_CONTROL_POINTS> patch)
{
    DomainShaderOutput output;
    
    // p(u,v) = (u,v,w)(p_0,p_1,p_2)^T      (1-u-v=w) are the barycentric coords.
    float3 linearPos = patch[0].worldPosition * barycentric.x + patch[1].worldPosition * barycentric.y + patch[2].worldPosition * barycentric.z;
    
    float3 phongPos = float3(0, 0, 0);
    
    for (int i = 0; i < NUM_CONTROL_POINTS; i++)
    {
        float3 pointToPos = linearPos - patch[i].worldPosition;
        float proj = dot(pointToPos, patch[i].normal);          
        
        // Projection: t_i(q) = q - ((q-p_i)*n_i
        float3 pointPlane = linearPos - proj * patch[i].normal;        
        phongPos += barycentric[i] * pointPlane;
    }
    

    output.worldPosition = float4(lerp(linearPos, phongPos, 0.7f), 1);
    output.normal = normalize(patch[0].normal * barycentric.x + patch[1].normal * barycentric.y + patch[2].normal * barycentric.z);
    output.uv = patch[0].uv * barycentric.x + patch[1].uv * barycentric.y + patch[2].uv * barycentric.z;
    output.position = mul(output.worldPosition, vp);
    return output;
};


/*
    * Plance f/e tri-point:
    * t_i(q) = q - ((q-p_i)*n_i)                                                // Projection of plane on vertex v_i
    * p*(u,vp) = (u,v,w) * (t_0(p(u,v)), t_1(p(u,v)), t_2(p(u,v)))^T      // Transformed pos

    * p_a^*(u,v) = (1-a)*p(u,v) + a*p^*(u_v)                                    // standard a=0.75

*/