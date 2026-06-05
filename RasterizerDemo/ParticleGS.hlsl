cbuffer Camera : register(b0)
{
    float4x4 vp;
    float3 position;
    float padding;
};

struct GeometryShaderOutput
{
    float4 pos : SV_POSITION;
    float4 normal : NORMAL;
    float2 uv : UV;
};
    //float4 color : COLOR;

[maxvertexcount(6)]
void main(
point float3 input[1] : POSITION,
inout TriangleStream<GeometryShaderOutput> output
)
{
    // Calculate front vector using input and cam pos
    float3 frontVec = normalize(position - input[0]);
    
    // Use front vec along w default up to calc r vec
        float3 rightVec = normalize(cross(frontVec, float3(0.0f, 1.0f, 0.0f)));
  
        // use front vec and right vec to calculate actual up vec
        float3 upVec = normalize(cross(frontVec, rightVec));
        
    // scale right and up at end based on preference
    rightVec *= 0.1;
    upVec *= 0.1;
    
    
    GeometryShaderOutput toAppend;
    
    // Top left
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), vp);
    toAppend.normal = float4(frontVec, 1.0f);
    //toAppend.color = float4(1, 0, 0, 1);
    toAppend.uv = float2(0, 0);
    
    // Bottom right
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), vp);
    toAppend.normal = float4(frontVec, 1.0f);
    //toAppend.color = float4(1, 0, 0, 1);
    toAppend.uv = float2(0, 1);
    
    // Bottom left
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), vp);
    toAppend.normal = float4(frontVec, 1.0f);
    //toAppend.color = float4(1, 0, 0, 1);
    toAppend.uv = float2(1, 1);
    
    output.RestartStrip();  // triangle one
    

    // Top left;
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), vp);
    toAppend.normal = float4(frontVec, 1.0f);
    //toAppend.color = float4(1, 0, 0, 1);
    toAppend.uv = float2(0, 0); 
    
    // Top right
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), vp);
    toAppend.normal = float4(frontVec, 1.0f);
    //toAppend.color = float4(1, 0, 0, 1);
    toAppend.uv = float2(1, 0);
    
    // Bottom right
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), vp);
    toAppend.normal = float4(frontVec, 1.0f);
    //toAppend.color = float4(1, 0, 0, 1);
    toAppend.uv = float2(1, 1);

    output.Append(toAppend);
}
