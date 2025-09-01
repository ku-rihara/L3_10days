
struct AmbientLight
{
    float4 color;
    float intensity;
};


float3 CalculateAmbientLight(AmbientLight light, float3 materialColor, float3 textureColor)
{
    return materialColor * textureColor * light.color.rgb * light.intensity;
}


float3 GetAmbientLightColor(AmbientLight light)
{
    return light.color.rgb * light.intensity;
}


float3 ApplyAmbientLight(AmbientLight light, float3 baseColor)
{
    return baseColor * light.color.rgb * light.intensity;
}
