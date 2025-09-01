#include "Boundary.hlsli"

struct PSOutput {
	float4 color : SV_Target0;
};

struct Hole {
	float3 position;
	float radius;
};


static const int kMaxHoles = 128;
StructuredBuffer<Hole> gHoles : register(t0);

PSOutput main(VertexShaderOutput input) {
	PSOutput output;

	/// holeとの距離を計算
	for (uint i = 0; i < 128; i++) {
		float distance = length(input.worldPosition - gHoles[i].position);
		if (distance < gHoles[i].radius) {
			discard;
		}
	}

	output.color = float4(1,0,0, 1.0f);
	return output;
}