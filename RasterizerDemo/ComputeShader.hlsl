RWTexture2D<unorm float4> backBufferUAV : register(u0);

SamplerState samplerState : register(s0);

Texture2D<float4> positionGBuffer : register(t0);
Texture2D<float4> normalGBuffer : register(t1);
Texture2D<float4> diffuseGBuffer : register(t2);

struct LightBuffer
{
    float4x4 vpmatrix;
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float angle;
};

StructuredBuffer<LightBuffer> SpotLights : register(t3);
StructuredBuffer<LightBuffer> DirLights : register(t4);


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
    int padding1;
    int padding2;
};


[numthreads(8, 8, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID)
{
    float4 pixelPosition = float4(positionGBuffer[DTid.xy].xyz, 1);     // Position
    float3 pixelNormal = normalGBuffer[DTid.xy].xyz;                    // Normal
    
    float ambientStandard = positionGBuffer[DTid.xy].a;
    float4 diffuse =  float4(0, 0, 0, 0);
    float4 specular = float4(0, 0, 0, 0);
    float specularKof = 100;
    
    /* Light Calculations using
        * Phong for diffuse 
            * Diffuse:  c_d = I * k_d * max(0, dot(n, l))
        *Blinn-Phong for 
            *Specular: c_s = I * k_s * pow(max(0, dot(n,h)), p)
    */
    
    // Light Calculations for SpotLights
    for (int i = 0; i < nrofSpotLights; i++)
    {
        LightBuffer spotlight = SpotLights[i];
        
        float3 L = normalize(spotlight.position - pixelPosition.xyz);   // LightDir
        float3 V = normalize(cameraPosition.xyz - pixelPosition.xyz);   // ViewDir
        float3 H = normalize(L + V);
        float T = dot(L, pixelNormal);                                  // Theta for diffuse  (LightDir & Normal)
        float S = saturate(dot(pixelNormal.xyz, H));                    // Sigma for specular (Halfway-Vector & Normal)
       
        float3 D = normalize(spotlight.direction);
        float cosT = dot(-L, D);
        float spoEffect = smoothstep(cos(spotlight.angle), 1.0f, cosT);
        
        // Setting the diffuse and specular    
        diffuse += spotlight.intensity * spotlight.color * float4(diffuseGBuffer[DTid.xy].xyz, 0) * saturate(T);
        specular += spotlight.intensity * spotlight.color * normalGBuffer[DTid.xy].a * pow(saturate(S), specularKof);
        
        if (spotlight.intensity > ambientStandard)
            ambientStandard = spotlight.intensity;
    }
    
    // Light Calculations for Directional Lights
    for (int j = 0; j < nrofDirLights; j++ )
    {
        LightBuffer dirlight = DirLights[j];
        
        float3 L = normalize(dirlight.direction.xyz);
        float3 V = normalize(cameraPosition.xyz - pixelPosition.xyz);
        float3 H = normalize(L);        // H = ( L + V ) / || L + V ||
        float T = dot(L, pixelNormal);
        float S = saturate(dot(pixelNormal.xyz, H));
        
        if(dirlight.intensity > ambientStandard)
            ambientStandard = dirlight.intensity;
        
        diffuse += dirlight.intensity * dirlight.color * float4(diffuseGBuffer[DTid.xy].xyz, 0) * saturate(T);// * float4(1, 1, 1, 0);
        specular += dirlight.color * dirlight.intensity * normalGBuffer[DTid.xy].a * pow(saturate(S), specularKof); // * float4(1, 1, 1, 0));
    }
    // c_a = k_a * l_a
    float4 ambient = positionGBuffer[DTid.xy].a *  float4(diffuseGBuffer[DTid.xy].xyz, 0);    

    backBufferUAV[DTid.xy] = ambient + diffuse + specular;
  
    return;
    
    
    
    /*
        * Ambient:  c_a = k_a * I_a 
        * Diffuse:  c_d = I * k_d * max(0, cos(a))
        * Specular:                                                                       [eggshell  mirror]
            * Phong:        c_s = I * k_s * pow(max(0, dot(n,l)), p)  // phong exp / shininess [ 100, 10000]
            * Blinn-Phong:  c_s = I * k_s * pow(max(0, dot(n,h)), p)
    
    
                                        cos(a) = dot(l,n)
        * Lambert's law:        I = max(0, dot(N,L))
                                L_diff = albedo (diffuse tror jag) * lightcolor * I
                                c_k = I * k_d * cos(a)^+
                                c_k = I * k_d * max(0, cos(a))
                                VAD ÄR I???? Angle of Incidence? INTENSITY!!!!
        * Blinn-Phong Specular: pow(max(0, dot(N,H), shininess)
    */
}
