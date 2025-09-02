#include "BoundaryShard.hlsli"
#include "ShadowMap/ShadowMap.hlsli"

struct TransformationMatrix {
	float4x4 WVP;
	float4x4 World;
	float4x4 WorldInverseTranspose;
};

struct InstanceData {
	int shardMaxSize; // 破片の最大数
};

StructuredBuffer<TransformationMatrix> gTransformations : register(t0);
ConstantBuffer<ShadowTransformBuffer> gShadowTransformBuffer : register(b0);
ConstantBuffer<InstanceData> gInstanceCount : register(b1);

VSOutput main(VSInput input, uint instanceId : SV_InstanceID) {
	VSOutput output;
	
	int id = instanceId + (instanceId * gInstanceCount.shardMaxSize);
	TransformationMatrix transformation = gTransformations[id];
	
	output.normal = normalize(mul(input.normal, (float3x3) transformation.WorldInverseTranspose));
	output.position = mul(input.position, transformation.WVP);
	output.texcoord = input.texcoord;
	float4 worldPos = mul(input.position, transformation.World);
	output.worldPosition = worldPos.xyz;
	output.tpos = mul(worldPos, gShadowTransformBuffer.lightCamera);
	return output;
}