cbuffer Camera : register(b0)
{
    float4x4 viewProj;
    float3 position;
    float padding;
}

cbuffer WorldMatrix : register(b1)
{
	float4x4 world;
};

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
	float2 uv : UV;
};

struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float4 worldPosition : WORLD_POSITION;
	float3 normal : NORMAL;
	float2 uv : UV;
};

float3x3 inverse(float3x3 m);

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.worldPosition = mul(float4(input.position, 1.0f), world);
	output.position = mul(output.worldPosition, viewProj);
	//output.normal = normalize(mul(float4(input.normal, 0.0f), world).xyz);
    output.normal = normalize(mul(input.normal, (float3x3)world));
    //float3x3 normalMatrix = (transpose(inverse((float3x3)world)));
    //output.normal = normalize(mul(input.normal, normalMatrix));
	
	output.uv = input.uv;
	return output;
}

float3x3 inverse(float3x3 m)
{
    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

    float b01 = a22 * a11 - a12 * a21;
    float b11 = -a22 * a10 + a12 * a20;
    float b21 = a21 * a10 - a11 * a20;

    float det = a00 * b01 + a01 * b11 + a02 * b21;

    // Avoid division by zero
    if (abs(det) < 1e-8)
        return float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);

    float invDet = 1.0 / det;

    float3x3 inv;
    inv[0][0] = b01 * invDet;
    inv[0][1] = (-a22 * a01 + a02 * a21) * invDet;
    inv[0][2] = (a12 * a01 - a02 * a11) * invDet;
    inv[1][0] = b11 * invDet;
    inv[1][1] = (a22 * a00 - a02 * a20) * invDet;
    inv[1][2] = (-a12 * a00 + a02 * a10) * invDet;
    inv[2][0] = b21 * invDet;
    inv[2][1] = (-a21 * a00 + a01 * a20) * invDet;
    inv[2][2] = (a11 * a00 - a01 * a10) * invDet;

    return inv;
}