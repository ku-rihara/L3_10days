#include "BoundaryShard.hlsli"

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

	output.color.rgb = CalculateLighting(input.normal);
	output.color.a = 1.0;

	return output;
}