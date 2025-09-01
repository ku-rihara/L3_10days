#include "Noise.hlsli"

struct VertexShaderOutput {
	float4 position      : SV_POSITION;
	float2 texcoord      : TEXCOORD0;
	float3 normal        : NORMAL0;
	float3 worldPosition : POSITION0;
    float4 tpos          : TPOS;
};

struct VertexShaderInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
};

struct Time {
	float value;
};

struct Hole {
	float3 position;
	float radius;
	float startRadius;
	float lifeTime;
};

float fbm(float2 uv) {
	float sum = 0.0;
	float amp = 0.5;
	float freq = 1.0;
	for (int i = 0; i < 5; i++) {
		sum += amp * PerlinNoise(uv * freq);
		freq *= 2.0;
		amp *= 0.5;
	}
	return sum;
}