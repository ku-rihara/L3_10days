struct VSInput {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal  : NORMAL;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET;
};

struct Time {
    float value;
};

float Rand2dTo1d(float2 uv) {
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}