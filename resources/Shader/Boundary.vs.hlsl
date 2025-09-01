#include "Boundary.hlsli"
#include "ShadowMap/ShadowMap.hlsli"

struct TransformationMatrix {
	float4x4 WVP;
	float4x4 World;
	float4x4 WorldInverseTranspose;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
ConstantBuffer<ShadowTransformBuffer> gShadowTransformBuffer : register(b1);

VertexShaderOutput main(VertexShaderInput input) {
	VertexShaderOutput output;
	output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
	output.position = mul(input.position, gTransformationMatrix.WVP);
	output.texcoord = input.texcoord;

	float4 worldPos = mul(input.position, gTransformationMatrix.World);
	output.worldPosition = worldPos.xyz;
	output.tpos = mul(worldPos, gShadowTransformBuffer.lightCamera);

	return output;
}