#include "MiniMap.hlsli"

struct Vertex {
    float4 pos;
    float2 uv;
};

static const Vertex gVertices[4] = {
    { float4(-1.0f, -1.0f, 0.0f, 1.0f), float2(0.0f, 0.0f) },
    { float4( 1.0f, -1.0f, 0.0f, 1.0f), float2(1.0f, 0.0f) },
    { float4(-1.0f,  1.0f, 0.0f, 1.0f), float2(0.0f, 1.0f) },
    { float4( 1.0f,  1.0f, 0.0f, 1.0f), float2(1.0f, 1.0f) },
};

/// C++側で設定する行列がスクリーン座標からNDCへの変換行列
static const float4x4 gScreenToNDC = float4x4(
	2.0f / 1280.0f, 0.0f,           0.0f, 0.0f,
	0.0f,           -2.0f / 720.0f, 0.0f, 0.0f,
	0.0f,           0.0f,           1.0f, 0.0f,
	-1.0f,          1.0f,           0.0f, 1.0f
);


VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;

    Vertex v = gVertices[vertexID];
    output.position = v.pos;
    output.texcoord = v.uv;

    // スクリーン座標をNDCに変換
    output.screenPosition = mul(v.pos, gMiniMapData.mat);
    output.position = mul(output.screenPosition, gScreenToNDC);

    return output;
}
