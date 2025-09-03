#include "MiniMapIcon.hlsli"


/// C++側で設定する行列がスクリーン座標からNDCへの変換行列
static const float4x4 gScreenToNDC = float4x4(
	2.0f / 1280.0f, 0.0f,           0.0f, 0.0f,
	0.0f,           -2.0f / 720.0f, 0.0f, 0.0f,
	0.0f,           0.0f,           1.0f, 0.0f,
	-1.0f,          1.0f,           0.0f, 1.0f
);

StructuredBuffer<Icon> gIcons : register(t0);

VSOutput main(VSInput input, uint instanceId : SV_InstanceID) {
	VSOutput output;
	output.position = mul(float4(input.position.xyz, 1.0), mul(gIcons[instanceId].mat, gScreenToNDC));
	output.texcoord = input.texcoord;
	return output;
}