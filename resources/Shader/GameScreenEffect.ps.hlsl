#include "GameScreenEffect.hlsli"

struct EffectBufData {
	float4 baseColor;
};

/// vignetteとrandom noiseで画面端に向かって点滅する警告を表示
ConstantBuffer<Time> gTime : register(b0);
ConstantBuffer<EffectBufData> gEffectBuf : register(b1);

float3 ApplyNoise(float2 uv, float3 color) {
	float2 center = float2(0.5, 0.5);
	float dist = distance(uv, center); // 0〜0.707（角までの距離）

	float minNoise = 0.2; // 中央でも最低20%ノイズ
	float maxNoise = 0.6; // 端のノイズ強度
	float noiseStrength = lerp(minNoise, maxNoise, dist / 0.707);

	/// 時間で揺れるノイズにする場合はuvにtimeを混ぜる
	float random = Rand2dTo1d(uv * 1000.0 + gTime.value);
	float finalNoise = random * noiseStrength;
	return color + finalNoise; // ノイズ加算
}

PSOutput main(VSOutput input) {
	PSOutput output;

	float3 color = ApplyNoise(input.texcoord, gEffectBuf.baseColor.rgb);
	output.color = float4(color, gEffectBuf.baseColor.a);

	return output;
}