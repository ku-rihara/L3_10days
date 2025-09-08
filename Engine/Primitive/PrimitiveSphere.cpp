#include "PrimitiveSphere.h"
#include "3d/Mesh.h"
#include <numbers>
#include <vector>

void PrimitiveSphere::Init() {
    const uint32_t kSubdivision = 16;
    // 球面の頂点数を計算
    vertexNum_ = kSubdivision * kSubdivision * 6;
    IPrimitive::Init();
}

void PrimitiveSphere::Create() {
    const uint32_t kSubdivision = 16;
    const float kRadius         = 1.0f;
    const float kLonEvery       = 2.0f * std::numbers::pi_v<float> / float(kSubdivision); // 経度の刻み
    const float kLatEvery       = std::numbers::pi_v<float> / float(kSubdivision); // 緯度の刻み

    uint32_t vertexIndex = 0;

    // 緯度の分割
    for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
        float lat     = -std::numbers::pi_v<float> / 2.0f + kLatEvery * latIndex;
        float latNext = -std::numbers::pi_v<float> / 2.0f + kLatEvery * (latIndex + 1);

        // 経度の分割
        for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
            float lon     = lonIndex * kLonEvery;
            float lonNext = (lonIndex + 1) * kLonEvery;

            // 4つの頂点の座標を計算
            // A: (lat, lon)
            float aX          = kRadius * std::cos(lat) * std::cos(lon);
            float aY          = kRadius * std::sin(lat);
            float aZ          = kRadius * std::cos(lat) * std::sin(lon);
            Vector3 aNormal   = Vector3(aX, aY, aZ);
            Vector2 aTexcoord = Vector2(float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision));

            // B: (lat, lonNext)
            float bX          = kRadius * std::cos(lat) * std::cos(lonNext);
            float bY          = kRadius * std::sin(lat);
            float bZ          = kRadius * std::cos(lat) * std::sin(lonNext);
            Vector3 bNormal   = Vector3(bX, bY, bZ);
            Vector2 bTexcoord = Vector2(float(lonIndex + 1) / float(kSubdivision), 1.0f - float(latIndex) / float(kSubdivision));

            // C: (latNext, lon)
            float cX          = kRadius * std::cos(latNext) * std::cos(lon);
            float cY          = kRadius * std::sin(latNext);
            float cZ          = kRadius * std::cos(latNext) * std::sin(lon);
            Vector3 cNormal   = Vector3(cX, cY, cZ);
            Vector2 cTexcoord = Vector2(float(lonIndex) / float(kSubdivision), 1.0f - float(latIndex + 1) / float(kSubdivision));

            // D: (latNext, lonNext)
            float dX          = kRadius * std::cos(latNext) * std::cos(lonNext);
            float dY          = kRadius * std::sin(latNext);
            float dZ          = kRadius * std::cos(latNext) * std::sin(lonNext);
            Vector3 dNormal   = Vector3(dX, dY, dZ);
            Vector2 dTexcoord = Vector2(float(lonIndex + 1) / float(kSubdivision), 1.0f - float(latIndex + 1) / float(kSubdivision));

            // 1つ目の三角形: A, B, C
            mesh_->SetVertexPositionData(vertexIndex, Vector4(aX, aY, aZ, 1.0f));
            mesh_->SetVertexTexcoordData(vertexIndex, aTexcoord);
            mesh_->SetVertexNormData(vertexIndex, aNormal);
            vertexIndex++;

            mesh_->SetVertexPositionData(vertexIndex, Vector4(bX, bY, bZ, 1.0f));
            mesh_->SetVertexTexcoordData(vertexIndex, bTexcoord);
            mesh_->SetVertexNormData(vertexIndex, bNormal);
            vertexIndex++;

            mesh_->SetVertexPositionData(vertexIndex, Vector4(cX, cY, cZ, 1.0f));
            mesh_->SetVertexTexcoordData(vertexIndex, cTexcoord);
            mesh_->SetVertexNormData(vertexIndex, cNormal);
            vertexIndex++;

            // 2つ目の三角形: C, B, D
            mesh_->SetVertexPositionData(vertexIndex, Vector4(cX, cY, cZ, 1.0f));
            mesh_->SetVertexTexcoordData(vertexIndex, cTexcoord);
            mesh_->SetVertexNormData(vertexIndex, cNormal);
            vertexIndex++;

            mesh_->SetVertexPositionData(vertexIndex, Vector4(bX, bY, bZ, 1.0f));
            mesh_->SetVertexTexcoordData(vertexIndex, bTexcoord);
            mesh_->SetVertexNormData(vertexIndex, bNormal);
            vertexIndex++;

            mesh_->SetVertexPositionData(vertexIndex, Vector4(dX, dY, dZ, 1.0f));
            mesh_->SetVertexTexcoordData(vertexIndex, dTexcoord);
            mesh_->SetVertexNormData(vertexIndex, dNormal);
            vertexIndex++;
        }
    }

    // インデックスデータ生成
    std::vector<uint32_t> indices(vertexNum_);
    for (uint32_t i = 0; i < vertexNum_; ++i) {
        indices[i] = i;
    }
    mesh_->SetIndexData(indices.data(), static_cast<uint32_t>(indices.size()));
}

void PrimitiveSphere::SetTexture(const std::string& name) {
    IPrimitive::SetTexture(name);
}

void PrimitiveSphere::Draw(const WorldTransform& worldTransform, const ViewProjection& viewProjection, std::optional<uint32_t> textureHandle) {
    IPrimitive::Draw(worldTransform, viewProjection, textureHandle);
}