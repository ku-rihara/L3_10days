#include "SkyDome.hlsli"
#include "Noise.hlsli"

Texture2D<float4>    gTexture : register(t0);
SamplerState         gSampler : register(s0);
ConstantBuffer<Time> gTime    : register(b0);

static const float2 uvScale = float2(20.0, 20.0);

PSOutput main(VSOutput input) {
    PSOutput output;

    float2 scaledUV = input.texcoord * uvScale;
    // Sample the texture using the provided texture coordinates
    float4 color = gTexture.Sample(gSampler, scaledUV);

    float alpha = PerlinNoise(scaledUV + gTime.value * 0.02);

    output.color = color * 0.75f;
    color.a = alpha;

    return output;
}