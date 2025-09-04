#include "PlayerOutsideWarning.hlsli"

/// vignetteとrandom noiseで画面端に向かって点滅する警告を表示

ConstantBuffer<Time> gTime : register(b1);

static const float3 gBaseColor = float3(1.0, 0.2, 0.2); // #ff3333

PSOutput main(VSOutput input)
{
    PSOutput output;

    output.color = float4(gBaseColor, 0.0f);

    /// 画面中央に向かって透明になるvignette
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    float vignette = correct.x * correct.y * 16.0f;
    vignette = saturate(pow(vignette, 0.8f));
    output.color.rgb *= vignette;

    /// ランダムノイズで点滅
    float noise = Rand2dTo1d(input.texcoord * 1000.0f + gTime.value * 0.5f);
    output.color.a = (1.0f - vignette) * noise;

    return output;
}