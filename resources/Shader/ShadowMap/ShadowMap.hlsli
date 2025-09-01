
struct ShadowTransformBuffer
{
    float4x4 lightCamera;
};

float CalculateShadow(float4 lightSpacePosition, Texture2D<float> ShadowMap, SamplerComparisonState shadowSampler)
{
   
    float3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    projCoords.y = 1.0 - projCoords.y; 
    
    float currentDepth = projCoords.z;
    

    float bias = 0.005;
    float depthWithBias = currentDepth - bias;
  
    float shadowFactor = ShadowMap.SampleCmp(shadowSampler, projCoords.xy, depthWithBias);
    
    return lerp(0.3, 1.0, shadowFactor);
}
