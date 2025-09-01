struct DirectionalLight
{
    float4 color;
    float3 direction; 
    float intensity; 
};


float3 CalculateLambertDiffuse(float3 normal, float3 lightDirection, float3 materialColor, float3 textureColor, float3 lightColor, float lightIntensity)
{
    float cos = saturate(dot(normalize(normal), -lightDirection));
    return materialColor * textureColor * lightColor * cos * lightIntensity;
}


float3 CalculateHalfLambertDiffuse(float3 normal, float3 lightDirection, float3 materialColor, float3 textureColor, float3 lightColor, float lightIntensity)
{
    float NdotL = dot(normalize(normal), -lightDirection);
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    return materialColor * textureColor * lightColor * cos * lightIntensity;
}


float3 CalculateSpecularReflection(float3 normal, float3 lightDirection, float3 toEye, float3 lightColor, float lightIntensity, float shininess)
{
    float3 halfVector = normalize(-lightDirection + toEye);
    float NdotH = dot(normalize(normal), halfVector);
    float specularPow = pow(saturate(NdotH), shininess);
    return lightColor * lightIntensity * specularPow;
}

float3 CalculateDirectionalLightDiffuse(DirectionalLight light, float3 normal, float3 materialColor, float3 textureColor)
{
    float NdotL = dot(normalize(normal), -light.direction);
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
    return materialColor * textureColor * light.color.rgb * cos * light.intensity;
}


float3 CalculateDirectionalLightSpecular(DirectionalLight light, float3 normal, float3 toEye, float shininess)
{
    float3 halfVector = normalize(-light.direction + toEye);
    float NdotH = dot(normalize(normal), halfVector);
    float specularPow = pow(saturate(NdotH), shininess);
    return light.color.rgb * light.intensity * specularPow;
}
