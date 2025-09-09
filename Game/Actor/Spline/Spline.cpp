#include "Spline.h"

// std
#include <filesystem>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <cmath>

// externals
#include <json.hpp>

Spline::Spline() {
    line3D_ = std::make_unique<Line3D>();
    line3D_->Init(static_cast<size_t>(std::pow(2.0, 12.0)));
}

Spline::~Spline() {}

/////////////////////////////////////////////////////////////////////////////////////////
//      JSONロード
/////////////////////////////////////////////////////////////////////////////////////////
void Spline::Load(const std::string& filepath) {
    if (!std::filesystem::exists(filepath)) return;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        assert(false && "Spline::Load: failed to open file");
        return;
    }

    nlohmann::json j;
    file >> j;

    controlPoints_.clear();
    controlPoints_.reserve(j.size());

    for (auto& v : j) {
        float bx = v[0].get<float>();
        float by = v[1].get<float>();
        float bz = v[2].get<float>();

        // Blender (X,Y,Z) -> DirectX (X,Z,Y)、右手系→左手系変換
        controlPoints_.push_back({ bx, by, bz });
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//      デバッグ描画
/////////////////////////////////////////////////////////////////////////////////////////
void Spline::DebugDraw(const ViewProjection& vp) {
    const size_t n = controlPoints_.size();
    if (n < 2) return;

    line3D_->Reset();

    const int kSegments = 64;
    for (int i = 0; i < static_cast<int>(n) - 1; ++i) {
        for (int j = 0; j < kSegments; ++j) {
            float u0 = static_cast<float>(j) / kSegments;
            float u1 = static_cast<float>(j + 1) / kSegments;

            Vector3 p0 = Evaluate((i + u0) / (static_cast<float>(n) - 1.0f));
            Vector3 p1 = Evaluate((i + u1) / (static_cast<float>(n) - 1.0f));

            line3D_->SetLine(p0, p1, { 1.0f, 0.0f, 0.0f, 1.0f });
        }
    }
    line3D_->Draw(vp);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      Catmull-Rom 補間
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 Spline::CatmullRom(const Vector3& p0, const Vector3& p1,
                           const Vector3& p2, const Vector3& p3,
                           float t) const {
    const float t2 = t * t;
    const float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      正規化サンプリング
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 Spline::Evaluate(float u) const {
    const size_t n = controlPoints_.size();
    if (n == 0) return {};
    if (n == 1) return controlPoints_[0];
    if (n == 2) {
        return controlPoints_[0] * (1.0f - u) + controlPoints_[1] * u;
    }

    int segCount = static_cast<int>(n) - 1;
    float f = u * segCount;
    int seg = static_cast<int>(f);
    float t = f - seg;

    seg = std::clamp(seg, 0, segCount - 1);

    int i0 = (std::max)(seg - 1, 0);
    int i1 = seg;
    int i2 = seg + 1;
    int i3 = (std::min)(seg + 2, segCount);

    return CatmullRom(controlPoints_[i0], controlPoints_[i1],
                      controlPoints_[i2], controlPoints_[i3], t);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      アクセサ
/////////////////////////////////////////////////////////////////////////////////////////
size_t Spline::GetControlPointCount() const { return controlPoints_.size(); }
const std::vector<Vector3>& Spline::GetControlPoints() const { return controlPoints_; }
std::vector<Vector3>& Spline::GetControlPointsMutable() { return controlPoints_; }
