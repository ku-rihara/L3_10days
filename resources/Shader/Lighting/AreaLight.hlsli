struct AreaLight
{
    float4 color; 
    float3 position; 
    float padding1;
    float3 normal;
    float intensity;
    float width; 
    float height; 
    float decay;
    float3 up;
    float padding2;
};

struct AreaLightRangeInfo
{
    bool isInRange;
    float3 localPoint;
    float x;
    float y;
    float falloffFactor;
};


float CalculateAreaLightDistanceAttenuation(float distance, float decay)
{
    return pow(saturate(-distance / 10.0f + 1.0f), decay);
}

AreaLightRangeInfo CheckAreaLightRange(AreaLight light, float3 worldPosition)
{
    AreaLightRangeInfo info;
    
   
    float3 toLight = light.position - worldPosition;
    
 
    float3 lightNormal = -normalize(light.normal);
    

    float dist = dot(toLight, lightNormal);
    float3 projectedPoint = worldPosition + dist * lightNormal;
    
  
    float3 right = normalize(cross(light.up, light.normal));
    float3 up = normalize(light.up);
    info.localPoint = projectedPoint - light.position;
    info.x = dot(info.localPoint, right);
    info.y = dot(info.localPoint, up);
    
   
    info.isInRange = (abs(info.x) <= light.width * 0.5f) && (abs(info.y) <= light.height * 0.5f);
    
    if (!info.isInRange)
    {
       
        float falloffRange = 1.0f;
        float edgeDistance = max(abs(info.x) - light.width * 0.5f, abs(info.y) - light.height * 0.5f);
        info.falloffFactor = saturate(1.0f - edgeDistance / falloffRange);
    }
    else
    {
        info.falloffFactor = 1.0f;
    }
    
    return info;
}


float3 CalculateAreaLightDiffuse(AreaLight light, float3 worldPosition, float3 normal, float3 materialColor, float3 textureColor)
{
    float3 toLight = light.position - worldPosition;
    float3 lightDir = normalize(light.position - worldPosition);
    float distance = length(toLight);
    
    float attenuationFactor = CalculateAreaLightDistanceAttenuation(distance, light.decay);
    float diffuse = saturate(dot(normalize(normal), -lightDir));
    
    return materialColor * textureColor * light.color.rgb * diffuse * light.intensity * attenuationFactor;
}

float3 CalculateAreaLightSpecular(AreaLight light, float3 worldPosition, float3 normal, float3 toEye, float shininess)
{
    float3 toLight = light.position - worldPosition;
    float3 lightDir = normalize(light.position - worldPosition);
    float distance = length(toLight);
    
    float attenuationFactor = CalculateAreaLightDistanceAttenuation(distance, light.decay);
    
    float3 halfVector = normalize(-lightDir + toEye);
    float specular = pow(saturate(dot(normalize(normal), halfVector)), shininess);
    
    return light.color.rgb * light.intensity * specular * attenuationFactor;
}


float3 CalculateAreaLightLighting(AreaLight light, float3 worldPosition, float3 normal, float3 toEye, float3 materialColor, float3 textureColor, float shininess)
{
    AreaLightRangeInfo rangeInfo = CheckAreaLightRange(light, worldPosition);
    
    float3 diffuse = CalculateAreaLightDiffuse(light, worldPosition, normal, materialColor, textureColor);
    float3 specular = CalculateAreaLightSpecular(light, worldPosition, normal, toEye, shininess);
    
    return (diffuse + specular) * rangeInfo.falloffFactor;
}
