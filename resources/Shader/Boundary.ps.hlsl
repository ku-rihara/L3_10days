#include "Boundary.hlsli"

struct PSOutput {
	float4 color : SV_Target0;
};

static const int kMaxHoles = 128;
StructuredBuffer<Hole> gHoles : register(t0);
ConstantBuffer<Time> gTime : register(b0);

/// 仮のベースとハイライトの色
static const float3 gBaseColor = float3(0.173, 0.886, 0.482); // #2de17b
static const float3 gHighlightColor = float3(0.173, 0.886, 0.647); // #2de1a5

PSOutput main(VertexShaderOutput input) {
	PSOutput output;

	for (uint i = 0; i < kMaxHoles; i++) {
		float distance = length(input.worldPosition - gHoles[i].position);
		if (distance < gHoles[i].radius) {
			discard;
		}
	}

	float2 baseColorUV = input.texcoord * 1.0 + gTime.value * 0.05;
	float baseColorValue = PerlinNoise(baseColorUV * 100);
	
	float2 highlightUV = input.texcoord.yx * 3.0 - gTime.value * 0.2;
	float highlightValue = fbm(highlightUV * 100);
	
	float alpha = PerlinNoise(baseColorUV) * PerlinNoise(highlightUV);
	alpha = clamp(alpha, 0.4f, 0.8f);

	float3 color = (gBaseColor * baseColorValue) * (gHighlightColor * highlightValue);
	output.color = float4(color, alpha);
	
	return output;
}
