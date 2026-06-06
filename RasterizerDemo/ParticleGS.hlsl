cbuffer Camera : register(b0)
{
    float4x4 viewProjMatrix;
    float3 cameraPosition;
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
    float3 frontVec = normalize(cameraPosition - input[0]);
    
    // Use front vec along w default up to calc r vec
    float3 rightVec = normalize(cross(float3(0.0f, 1.0f, 0.0f), frontVec));
  
    // use front vec and right vec to calculate actual up vec
    float3 upVec = normalize(cross(frontVec, rightVec));
        
    // scale right and up at end based on preference
    float size = 0.2f;
    rightVec *= size;
    upVec *= size;
    
    
    GeometryShaderOutput toAppend;
    
    // Tri 1
    // Top left
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), viewProjMatrix);
    toAppend.normal = float4(frontVec, 0.0f);
    toAppend.uv = float2(0, 0);
    output.Append(toAppend);
    
    // Bottom left
    toAppend.pos = mul(float4(input[0] - rightVec - upVec, 1.0f), viewProjMatrix);
    toAppend.normal = float4(frontVec, 0.0f);
    toAppend.uv = float2(0, 1);
    output.Append(toAppend);
    
    // Bottom right
    toAppend.pos = mul(float4(input[0] + rightVec - upVec, 1.0f), viewProjMatrix);
    toAppend.normal = float4(frontVec, 0.0f);
    toAppend.uv = float2(1, 1);
    output.Append(toAppend);
    
    
    output.RestartStrip();  // triangle one
    
    // Tri 2
    // Top left;
    toAppend.pos = mul(float4(input[0] - rightVec + upVec, 1.0f), viewProjMatrix);
    toAppend.normal = float4(frontVec, 0.0f);
    toAppend.uv = float2(0, 0); 
    output.Append(toAppend);
    
    
    // Bottom right
    toAppend.pos = mul(float4(input[0] + rightVec - upVec, 1.0f), viewProjMatrix);
    toAppend.normal = float4(frontVec, 0.0f);
    toAppend.uv = float2(1, 1);
    output.Append(toAppend);
    // Top right
    toAppend.pos = mul(float4(input[0] + rightVec + upVec, 1.0f), viewProjMatrix);
    toAppend.normal = float4(frontVec, 0.0f);
    toAppend.uv = float2(1, 0);
    output.Append(toAppend);
}
