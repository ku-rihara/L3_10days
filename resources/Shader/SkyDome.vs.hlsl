#include "SkyDome.hlsli"

struct MatrixBuffer {
    float4x4 matWVP;
};

ConstantBuffer<MatrixBuffer> gWorldViewProj : register(b0);

VSOutput main(VSInput input) {
    VSOutput output;

    // Transform the vertex position to clip space
    output.position = mul(input.position, gWorldViewProj.matWVP);
    // Pass through the texture coordinates
    output.texcoord = input.texcoord;

    return output;
}