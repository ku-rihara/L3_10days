#include "Boundary.hlsli"

struct PSOutput {
	float4 color : SV_Target0;
};


PSOutput main(VertexShaderOutput input) {
	PSOutput output;
	// 法線を0~1に変換
	//float3 normal = normalize(input.normal) * 0.5 + 0.5;
	// 出力する色を決定
	output.color = float4(1,0,0, 1.0f);
	return output;
}