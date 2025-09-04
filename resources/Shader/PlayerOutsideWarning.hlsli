struct VSOutput {
    float4 position       : SV_POSITION;
    float4 screenPosition : POSITION0;
    float2 texcoord       : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};


struct Time {
    float value;
};

//乱数
float rand2dTo1d(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}