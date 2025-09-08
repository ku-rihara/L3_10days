#include "MiniMapIcon.hlsli"

struct MiniMapData{
	float2 position;
	float radius;
};

ConstantBuffer<MiniMapData> gMiniMapSize : register(b0);
Texture2D<float4> gTexture : register(t1);
SamplerState gSampler : register(s0);


PSOutput main(VSOutput input) {
	PSOutput output;
	
	float distance = length(input.screenPosition.xy - gMiniMapSize.position);
	if(distance > gMiniMapSize.radius) {
		discard;
	}

	float4 texColor = gTexture.Sample(gSampler, input.texcoord);
	output.color = texColor;
	if(output.color.a < 0.1) {
		discard;
	}
	
	return output;
}