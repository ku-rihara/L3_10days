#include "BoundaryShard.hlsli"
#include "Noise.hlsli"

/// 仮想ライトと計算
static const float3 gLightDirection = normalize(float3(1, -1, 1));
static const float3 gLightColor = float3(1, 1, 1);
static const float3 gAmbientColor = float3(0.1, 0.1, 0.1);

float3 CalculateLighting(float3 normal) {
	float NdotL = max(dot(normal, -gLightDirection), 0.0);
	float3 diffuse = gLightColor * NdotL;
	return diffuse + gAmbientColor;
}

PSOutput main(VSOutput input) {
	PSOutput output;

	float noise = PerlinNoise(input.texcoord * 10);
	output.color.rgb = float3(0.855, 0.867, 0.173) * CalculateLighting(input.normal) * noise;
	output.color.a = noise;

	return output;
}