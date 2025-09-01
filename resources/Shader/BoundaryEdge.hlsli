struct VSInput {
	float4 position : POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL;
};

struct VSOutput {
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD0;
	float3 worldPosition : TEXCOORD1;
	float3 normal : TEXCOORD2;
	float4 tpos : TPOS;
};

struct PSOutput {
	float4 color : SV_Target0;
};