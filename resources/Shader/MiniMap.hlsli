#include "Noise.hlsli"

struct VSOutput {
    float4 position       : SV_POSITION;
    float4 screenPosition : POSITION0;
    float2 texcoord       : TEXCOORD0;
};

struct PSOutput {
    float4 color : SV_TARGET0;
};

struct MiniMapMatrix {
    float4x4 mat;
    float2 position;
	float radius;
};

struct Hole {
	float3 position;
	float radius;
	float startRadius;
	float lifeTime;
};

struct Breakable {
	float3 position;
	float radius;
};

struct Time {
	float value;
};

struct PlayerData {
    float3 position;
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

ConstantBuffer<MiniMapMatrix> gMiniMapData : register(b0);
