#include "MiniMap.hlsli"

/// ミニマップの色、ハイライト色
static const float3 gBaseColor = float3(0.173, 0.886, 0.482); // #2de17b
static const float3 gHighlightColor = float3(0.173, 0.886, 0.647); // rgba(51, 175, 134, 1)

ConstantBuffer<Time> gTime : register(b1);
ConstantBuffer<PlayerData> gPlayerData : register(b2);

PSOutput main(VSOutput input)
{
    PSOutput output;
    
    /// 範囲外なら透明にする
    float distance = length(input.screenPosition.xy - gMiniMapData.position);
    if (distance > gMiniMapData.radius) {
        discard;
    }

    /// プレイヤーの座標からuvを計算
    float2 ltPos = gPlayerData.position.xz + float2(-300.0f, 300.0f); // ミニマップの左上座標（ワールド座標）
    float2 rbPos = gPlayerData.position.xz + float2(300.0f, -300.0f); // ミニマップの右下座標（ワールド座標）
    float2 uv = (gPlayerData.position.xy - ltPos) / (rbPos - ltPos);
    uv = clamp(uv, 0.0f, 1.0f);

    float2 texcoord = input.texcoord;
    /// ミニマップの模様を生成
	float2 baseColorUV = texcoord * 1.0 + gTime.value * 0.05;
	float baseColorValue = PerlinNoise(baseColorUV * 100);
	float2 highlightUV = texcoord.yx * 3.0 - gTime.value * 0.2;
	float highlightValue = fbm(highlightUV * 100);
	float alpha = PerlinNoise(baseColorUV) * PerlinNoise(highlightUV);
	alpha = clamp(alpha, 0.4f, 0.8f);
	float3 color = (gBaseColor * baseColorValue) * (gHighlightColor * highlightValue);
	output.color = float4(color, alpha * 2.0f);


    return output;
}