struct SpotLight
{
    float4 color;    
    float3 position; 
    float intensity; 
    float3 direction;
    float distance;  
    float decay;     
    float cosAngle;  
    float cosFalloffStart;
};


float CalculateSpotLightDistanceAttenuation(float3 lightPosition, float3 worldPosition, float maxDistance, float decay)
{
    float distance = length(lightPosition - worldPosition);
    return pow(saturate(-distance / maxDistance + 1.0f), decay);
}


float CalculateSpotLightAngleAttenuation(float3 lightDirection, float3 spotDirection, float cosAngle, float cosFalloffStart)
{
    float currentCosAngle = dot(lightDirection, spotDirection);
    return saturate((currentCosAngle - cosAngle) / (cosFalloffStart - cosAngle));
}

float3 CalculateSpotLightDiffuse(SpotLight light, float3 worldPosition, float3 normal, float3 materialColor, float3 textureColor)
{
    float3 spotLightDirectionSurface = normalize(worldPosition - light.position);
    
    
    float attenuationFactor = CalculateSpotLightDistanceAttenuation(light.position, worldPosition, light.distance, light.decay);
  
    float NdotLSpot = dot(normalize(normal), -spotLightDirectionSurface);
    float cosSpot = saturate(NdotLSpot);
    
  
    float falloffFactor = CalculateSpotLightAngleAttenuation(spotLightDirectionSurface, light.direction, light.cosAngle, light.cosFalloffStart);
    
    return materialColor * textureColor * light.color.rgb * cosSpot * light.intensity * attenuationFactor * falloffFactor;
}


float3 CalculateSpotLightSpecular(SpotLight light, float3 worldPosition, float3 normal, float3 toEye, float shininess)
{
    float3 spotLightDirectionSurface = normalize(worldPosition - light.position);
    
   
    float attenuationFactor = CalculateSpotLightDistanceAttenuation(light.position, worldPosition, light.distance, light.decay);
    
  
    float3 halfVectorSpot = normalize(-spotLightDirectionSurface + toEye);
    float NdotHSpot = dot(normalize(normal), halfVectorSpot);
    float specularPowSpot = pow(saturate(NdotHSpot), shininess);
    
  
    float falloffFactor = CalculateSpotLightAngleAttenuation(spotLightDirectionSurface, light.direction, light.cosAngle, light.cosFalloffStart);
    
    return light.color.rgb * light.intensity * specularPowSpot * attenuationFactor * falloffFactor;
}


float3 CalculateSpotLightLighting(SpotLight light, float3 worldPosition, float3 normal, float3 toEye, float3 materialColor, float3 textureColor, float shininess)
{
    float3 diffuse = CalculateSpotLightDiffuse(light, worldPosition, normal, materialColor, textureColor);
    float3 specular = CalculateSpotLightSpecular(light, worldPosition, normal, toEye, shininess);
    return diffuse + specular;
}