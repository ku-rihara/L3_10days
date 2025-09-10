#include "PlayerOutOfFieldWarningEffect.hlsli"

struct EffectBufData {
    float4 baseColor;
};

ConstantBuffer<Time> gTime : register(b0);
ConstantBuffer<EffectBufData> gEffectBuf : register(b1);

PSOutput main(VSOutput input) {
    PSOutput output;

    // --- UV座標を -1 ~ 1 に（中心=0）
    float2 uv = input.texcoord * 2.0 - 1.0;
    float dist = length(uv);

    // --- vignette: 外に行くほど強くなる
    float vignette = smoothstep(0.5, 1.0, dist);

    // --- 点滅（sin波で点滅）
    float blink = abs(sin(gTime.value * 6.0));

    // --- ランダムノイズ
    float noise = frac(sin(dot(input.screenPosition.xy, float2(12.9898, 78.233))) * 43758.5453);

    // --- 外側だけに出るように合成
    float alpha = vignette * blink * noise;

    float3 color = gEffectBuf.baseColor.rgb;
    output.color = float4(color, alpha);

    return output;
}
