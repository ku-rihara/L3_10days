#include "MiniMapIcon.hlsli"

StructuredBuffer<Icon> gIcons : register(t0);

VSOutput main(VSInput input, uint instanceId : SV_InstanceID) {
	VSOutput output;
	output.position = mul(float4(input.position.xyz, 1.0), gIcons[instanceId].mat);
	output.texcoord = input.texcoord;
	return output;
}