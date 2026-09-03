RWTexture2D<unorm float4> backBufferUAV : register(u0);

SamplerState shadowMapSampler : register(s0);

Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> normalGBuffer : register(t1);
Texture2D<float4> diffuseGBuffer : register(t2);

Texture2DArray<float> spotLightShadowMap : register(t3);
Texture2DArray<float> dirLightShadowMap : register(t4);


struct LightBuffer
{
    float4x4 vpmatrix;
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float angle;
};

StructuredBuffer<LightBuffer> SpotLights : register(t5);
StructuredBuffer<LightBuffer> DirLights : register(t6);


cbuffer CameraBuffer : register(b0)
{
    float4x4 viewProjMatrix;
    float3 cameraPosition;
    float padding;
};


cbuffer NrOfLights : register(b1)
{
    int nrofSpotLights;
    int nrofDirLights;
    int shadowMappingOn;
    int padding2;
};

cbuffer RenderMode : register(b10)
{
    int renderingMode;
    float3 padding3;
}

static const int renderingModeStandard = 0;
static const int renderingModePosition = 1;
static const int renderingModeNormals = 2;
static const int renderingModeDiffuse = 3;
static const int renderingModeAmbient = 4;
static const int renderingModeSpecular = 5;

[numthreads(8, 8, 1)] 
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    if (renderingMode == renderingModeStandard)
    {
        float4 pixelPosition = float4(positionGBuffer[DTid.xy].xyz, 1); // Position
        float3 pixelNormal = normalGBuffer[DTid.xy].xyz; // Normal
    
        float ambientStandard = positionGBuffer[DTid.xy].a;
        float specularCoef = normalGBuffer[DTid.xy].a;
        float shininess = diffuseGBuffer[DTid.xy].a;
        float4 diffuse = float4(0, 0, 0, 0);
        float4 specular = float4(0, 0, 0, 0);
    
        /* Light Calculations using
            * Phong for diffuse 
                * Diffuse:  c_d = I * k_d * max(0, dot(n, l))
            * Blinn-Phong for 
                * Specular: c_s = I * k_s * pow(max(0, dot(n,h)), p)
        */
    
        // Light Calculations for SpotLights
        for (int i = 0; i < nrofSpotLights; i++)
        {
            LightBuffer spotlight = SpotLights[i];
        
            float3 pos = spotlight.position;
            float3 L = normalize(spotlight.position - pixelPosition.xyz); // LightDir
            float3 V = normalize(cameraPosition.xyz - pixelPosition.xyz); // ViewDir
            float3 H = normalize(L + V);
            float T = dot(L, pixelNormal); // Theta for diffuse  (LightDir & Normal)
            float S = max(0, dot(pixelNormal.xyz, H)); // Sigma for specular (Halfway-Vector & Normal)
       
            float3 D = normalize(spotlight.direction);
            float cosT = dot(-L, D);
            float spoEffect = smoothstep(cos(spotlight.angle), 1.0f, cosT);
            
            float4 lighting = spoEffect * spotlight.intensity * spotlight.color;
        
            float shadowFactor = 1;
            if (shadowMappingOn == 1)
            { 
                // ShadowMapping
                float4 clipSpace = mul(pixelPosition, spotlight.vpmatrix); // -> Clip Space
                float3 ndcSpace = (clipSpace.xyz / clipSpace.w); // -> NDC Space (Normalized Device Coodrinates)
        
                float3 shadowMapUV = float3(ndcSpace.x * 0.5f + 0.5f, ndcSpace.y * -0.5f + 0.5f, i); // Transforming to UV coodinates
                float shadowMapDepth = spotLightShadowMap.SampleLevel(shadowMapSampler, shadowMapUV, 0.0f).r + 0.00001; // Avoid self-shadowing
                shadowFactor = (ndcSpace.z > shadowMapDepth) ? 0.0f : 1.0f;
            }
        
            // Adding diffuse and specular    
            diffuse += lighting * shadowFactor  * float4(diffuseGBuffer[DTid.xy].xyz, 0) * saturate(T);
            specular += lighting * shadowFactor * specularCoef * pow(saturate(S), shininess); 
        
            if (spotlight.intensity > ambientStandard)
                ambientStandard = spotlight.intensity;
        }
    
        // Light Calculations for Directional Lights
        for (int j = 0; j < nrofDirLights; j++)
        {
            LightBuffer dirlight = DirLights[j];
            float4 lighting = dirlight.intensity * dirlight.color;
            
            float3 L = normalize(dirlight.direction.xyz);
            float3 V = normalize(cameraPosition.xyz - pixelPosition.xyz);
            float3 H = normalize(L + V); // H = ( L + V ) / || L + V ||
            float T = dot(L, pixelNormal);
            float S = dot(pixelNormal.xyz, H);
        
            float shadowFactor = 1;
            if (shadowMappingOn == 1)
            { 
                // ShadowMapping
                float4 clipSpace = mul(pixelPosition, dirlight.vpmatrix); // -> Clip Space
                float3 ndcSpace = (clipSpace.xyz / clipSpace.w); // -> NDC Space (Normalized Device Coodrinates)
        
                float3 shadowMapUV = float3(ndcSpace.x * 0.5f + 0.5f, ndcSpace.y * -0.5f + 0.5f, j); // Transforming to UV coodinates
                float shadowMapDepth = dirLightShadowMap.SampleLevel(shadowMapSampler, shadowMapUV, 0.0f).r + 0.01; // Avoid self-shadowing
                shadowFactor = (ndcSpace.z > shadowMapDepth) ? 0.0f : 1.0f;
            }
            // Setting a,d,s values
            if (dirlight.intensity > ambientStandard)
                ambientStandard = dirlight.intensity;
        
            diffuse += shadowFactor * lighting * float4(diffuseGBuffer[DTid.xy].xyz, 0) * saturate(T);
            specular += shadowFactor * lighting * specularCoef * pow(saturate(S), shininess); 
        }
        // c_a = k_a * l_a
        float4 ambient = ambientStandard * float4(diffuseGBuffer[DTid.xy].xyz, 0);

        backBufferUAV[DTid.xy] = ambient + diffuse + specular;
    }
    
    else if (renderingMode == renderingModePosition)
    {
        backBufferUAV[DTid.xy] = normalize(positionGBuffer[DTid.xy]);
    }
    else if (renderingMode == renderingModeNormals)
    {
        backBufferUAV[DTid.xy] = normalGBuffer[DTid.xy];
    }
    else if (renderingMode == renderingModeAmbient)
    {
        backBufferUAV[DTid.xy] = positionGBuffer[DTid.xy].w;
    }
    else if (renderingMode == renderingModeDiffuse)
    {
        backBufferUAV[DTid.xy] = diffuseGBuffer[DTid.xy];
    }
    else if (renderingMode == renderingModeSpecular)
    {
        backBufferUAV[DTid.xy] = normalGBuffer[DTid.xy].w;
    }
    
  
    return;
    
    /*
        * Ambient:  c_a = k_a * I_a 
        * Diffuse:  c_d = I * k_d * max(0, cos(a))
        * Specular:                                                                       [eggshell, mirror]
            * Phong:        c_s = I * k_s * pow(max(0, dot(n,l)), p)  // phong exp / shininess [ 100, 10000]
            * Blinn-Phong:  c_s = I * k_s * pow(max(0, dot(n,h)), p)
    
    
                                        cos(a) = dot(l,n)
        * Lambert's law:        I = max(0, dot(N,L))
                                L_diff = albedo * lightcolor * I
                                c_k = I * k_d * cos(a)^+
                                c_k = I * k_d * max(0, cos(a))
        * Blinn-Phong Specular: pow(max(0, dot(N,H), shininess)
    */
}
