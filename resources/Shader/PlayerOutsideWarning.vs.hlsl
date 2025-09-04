#include "PlayerOutsideWarning.hlsli"

struct Vertex {
    float4 position;
    float2 texcoord;
};

static const Vertex gVertices[4] = {
    { float4(-1.0f,  1.0f, 0.0f, 1.0f), float2(0.0f, 0.0f) },
    { float4( 1.0f,  1.0f, 0.0f, 1.0f), float2(1.0f, 0.0f) },
    { float4(-1.0f, -1.0f, 0.0f, 1.0f), float2(0.0f, 1.0f) },
    { float4( 1.0f, -1.0f, 0.0f, 1.0f), float2(1.0f, 1.0f) },
};

/// C++側で設定する行列がスクリーン座標からNDCへの変換行列
static const float4x4 gScreenToNDC = float4x4(
	2.0f / 1280.0f, 0.0f,           0.0f, 0.0f,
	0.0f,           -2.0f / 720.0f, 0.0f, 0.0f,
	0.0f,           0.0f,           1.0f, 0.0f,
	-1.0f,          1.0f,           0.0f, 1.0f
);


VSOutput main(uint vertexId : SV_VertexID)
{
    VSOutput output;
    output.position = gVertices[vertexId].position;
    output.screenPosition = output.position * 0.5f + float4(0.5f, 0.5f, 0.0f, 0.0f);
    output.texcoord = gVertices[vertexId].texcoord;
    return output;
}