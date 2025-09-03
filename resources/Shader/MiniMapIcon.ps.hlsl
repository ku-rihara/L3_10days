#include "MiniMapIcon.hlsli"

struct MiniMapSize {
	float2 min;
	float2 max;
};

ConstantBuffer<MiniMapSize> gMiniMapSize : register(b0);
Texture2D<float4> gTexture : register(t1);
SamplerState gSampler : register(s0);


PSOutput main(VSOutput input) {
	PSOutput output;
	
	//float2 uv = (input.texcoord - float2(0.5, 0.5)) * 2.0;
	//uv.y *= -1.0;
	
	//float2 size = gMiniMapSize.max - gMiniMapSize.min;
	//float aspect = size.x / size.y;
	//uv.x *= aspect;
	
	//float dist = length(uv);
	//if (dist > 1.0) {
	//	discard;
	//}

	float4 texColor = gTexture.Sample(gSampler, input.texcoord);
	output.color = texColor;
	if(output.color.a < 0.1) {
		discard;
	}
	
	return output;
}