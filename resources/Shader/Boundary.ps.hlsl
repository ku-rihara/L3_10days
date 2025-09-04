#include "Boundary.hlsli"

struct PSOutput {
	float4 color : SV_Target0;
};

static const int kMaxHoles = 128;
StructuredBuffer<Hole> gHoles : register(t0);
StructuredBuffer<Breakable> gBreakable: register(t1);
ConstantBuffer<Time> gTime : register(b0);

/// 仮のベースとハイライトの色
static const float3 gBaseColor = float3(0.173, 0.886, 0.482); // #2de17b
static const float3 gHighlightColor = float3(0.173, 0.886, 0.647); // #2de1a5

PSOutput main(VertexShaderOutput input) {
	PSOutput output;

	uint i = 0;
	for (i = 0; i < kMaxHoles; i++) {
		float distance = length(input.worldPosition - gHoles[i].position);
		if (distance < gHoles[i].radius) {
			discard;
		}
	}
	
	for(i = 0; i < kMaxHoles; i++) {
		float distance = length(input.worldPosition - gBreakable[i].position);
		if (distance < gBreakable[i].radius) {
			discard;
		}
	}

	float2 baseColorUV = input.texcoord * 1.0 + gTime.value * 0.005;
	float baseColorValue = PerlinNoise(baseColorUV * 100);
	
	float2 highlightUV = input.texcoord.yx * 3.0 - gTime.value * 0.02;
	float highlightValue = fbm(highlightUV * 100);
	
	float alpha = PerlinNoise(baseColorUV) * PerlinNoise(highlightUV);
	alpha = clamp(alpha, 0.4f, 0.8f);

	float3 color = (gBaseColor * baseColorValue) * (gHighlightColor * highlightValue);
		
	// --- 高さの元にする値（baseColorValueやhighlightValueを混ぜてもOK）
	float height = baseColorValue * 0.5 + highlightValue * 0.5;
	
	// --- サンプリング用オフセット
	float2 texelSize = float2(1.0 / 1024.0, 1.0 / 1024.0); // ミニマップなら適当な解像度に合わせる
	
	// --- 微小差分で高さを取得
	float hL = PerlinNoise((baseColorUV - float2(texelSize.x, 0)) * 100);
	float hR = PerlinNoise((baseColorUV + float2(texelSize.x, 0)) * 100);
	float hD = PerlinNoise((baseColorUV - float2(0, texelSize.y)) * 100);
	float hU = PerlinNoise((baseColorUV + float2(0, texelSize.y)) * 100);
	
	// --- 微分ベクトル（勾配）
	float dx = hR - hL;
	float dy = hU - hD;
	
	// --- 疑似法線を作成
	float3 normal = normalize(float3(-dx, -dy, 1.0));
	
	// --- 最終的にライティングに反映
	float3 lightDir = normalize(float3(0.3, 0.5, 0.8));
	float NdotL = saturate(dot(normal, lightDir));
	
	float3 shadedColor = color * NdotL;
	output.color = float4(shadedColor, alpha);

	
	return output;
}
