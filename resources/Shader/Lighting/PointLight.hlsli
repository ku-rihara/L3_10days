
struct PointLight
{
    float4 color;    
    float3 position; 
    float intenesity;
    float radius;    
    float decay;     
};


float CalculatePointLightAttenuation(float3 lightPosition, float3 worldPosition, float radius, float decay)
{
    float distance = length(lightPosition - worldPosition);
    return pow(saturate(-distance / radius + 1.0f), decay);
}


float3 CalculatePointLightDiffuse(PointLight light, float3 worldPosition, float3 normal, float3 materialColor, float3 textureColor)
{
    float3 pointLightDirection = normalize(worldPosition - light.position);
    float factorPoint = CalculatePointLightAttenuation(light.position, worldPosition, light.radius, light.decay);
    float NdotLPoint = dot(normalize(normal), -pointLightDirection);
    float cosPoint = saturate(NdotLPoint);
    
    return materialColor * textureColor * light.color.rgb * cosPoint * light.intenesity * factorPoint;
}


float3 CalculatePointLightSpecular(PointLight light, float3 worldPosition, float3 normal, float3 toEye, float shininess)
{
    float3 pointLightDirection = normalize(worldPosition - light.position);
    float factorPoint = CalculatePointLightAttenuation(light.position, worldPosition, light.radius, light.decay);
    float3 halfVectorPoint = normalize(-pointLightDirection + toEye);
    float NdotHPoint = dot(normalize(normal), halfVectorPoint);
    float specularPowPoint = pow(saturate(NdotHPoint), shininess);
    
    return light.color.rgb * light.intenesity * specularPowPoint * factorPoint;
}

float3 CalculatePointLightLighting(PointLight light, float3 worldPosition, float3 normal, float3 toEye, float3 materialColor, float3 textureColor, float shininess)
{
    float3 diffuse = CalculatePointLightDiffuse(light, worldPosition, normal, materialColor, textureColor);
    float3 specular = CalculatePointLightSpecular(light, worldPosition, normal, toEye, shininess);
    return diffuse + specular;
}