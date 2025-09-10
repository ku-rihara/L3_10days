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
        const float bx = v[0].get<float>();
        const float by = v[1].get<float>();
        const float bz = v[2].get<float>();
        // ここではそのまま (bx,by,bz) を採用
        controlPoints_.push_back({ bx, by, bz });
    }

    // ★ 新規ロードしたので弧長テーブルは無効化
    InvalidateArcLength_();
}

/////////////////////////////////////////////////////////////////////////////////////////
//      デバッグ描画
/////////////////////////////////////////////////////////////////////////////////////////
void Spline::DebugDraw(const ViewProjection& vp) {
    const size_t n = controlPoints_.size();
    if (n < 2) return;

    line3D_->Reset();

    // 弧長テーブルが無ければ簡易描画のために作る（省略可）
    if (!arcBuilt_) {
        BuildArcLengthTable(200);
    }

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

    // 範囲外はクランプ
    if (u <= 0.0f) return controlPoints_.front();
    if (u >= 1.0f) return controlPoints_.back();

    if (n == 2) {
        return controlPoints_[0] * (1.0f - u) + controlPoints_[1] * u;
    }

    const int segCount = static_cast<int>(n) - 1;
    float f = u * segCount;
    int seg = static_cast<int>(std::floor(f));
    float t = f - seg;

    seg = std::clamp(seg, 0, segCount - 1);

    const int i0 = (std::max)(seg - 1, 0);
    const int i1 = seg;
    const int i2 = seg + 1;
    const int i3 = (std::min)(seg + 2, segCount);

    return CatmullRom(controlPoints_[i0], controlPoints_[i1],
                      controlPoints_[i2], controlPoints_[i3], t);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      接線（微分の近似）
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 Spline::Tangent(float u) const {
    // 数値微分（中央差分）
    const float eps = 1e-3f;
    const float u0 = std::clamp(u - eps, 0.0f, 1.0f);
    const float u1 = std::clamp(u + eps, 0.0f, 1.0f);
    Vector3 d = Evaluate(u1) - Evaluate(u0);
    const float len = d.Length();
    return (len > 1e-6f) ? (d / len) : Vector3{0,0,1};
}

/////////////////////////////////////////////////////////////////////////////////////////
//      アクセサ
/////////////////////////////////////////////////////////////////////////////////////////
size_t Spline::GetControlPointCount() const { return controlPoints_.size(); }
const std::vector<Vector3>& Spline::GetControlPoints() const { return controlPoints_; }
std::vector<Vector3>& Spline::GetControlPointsMutable() {
    // CP を編集したら弧長テーブルは無効化
    InvalidateArcLength_();
    return controlPoints_;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      弧長テーブル
/////////////////////////////////////////////////////////////////////////////////////////
void Spline::BuildArcLengthTable(int samples) {
    const size_t n = controlPoints_.size();
    arcLength_.clear();
    totalLength_ = 0.0f;
    arcBuilt_ = false;

    if (n < 2 || samples < 1) {
        return;
    }

    arcLength_.resize(samples + 1);
    arcLength_[0] = 0.0f;

    Vector3 prev = Evaluate(0.0f);
    float accum = 0.0f;

    for (int i = 1; i <= samples; i++) {
        const float t = static_cast<float>(i) / samples;
        Vector3 p = Evaluate(t);
        accum += (p - prev).Length();
        arcLength_[i] = accum;
        prev = p;
    }
    totalLength_ = accum;
    arcBuilt_ = true;
}

float Spline::GetParamByDistance(float dist) const {
    // 弧長テーブルが無ければ安全に 0 を返す（呼び出し側で Build するか、遅延Buildへ変更可）
    if (!arcBuilt_ || arcLength_.size() < 2 || totalLength_ <= 0.0f) {
        return 0.0f;
    }

    // wrap（負値にも対応）
    float L = totalLength_;
    if (L <= 0.0f) return 0.0f;
    float m = std::fmod(dist, L);
    if (m < 0.0f) m += L; // [-L,0) → [0,L)

    // 二分探索
    int low = 0, high = static_cast<int>(arcLength_.size()) - 1;
    while (low < high) {
        int mid = (low + high) / 2;
        if (arcLength_[mid] < m) low = mid + 1;
        else high = mid;
    }
    const int i1 = std::clamp(low, 1, static_cast<int>(arcLength_.size()) - 1);
    const int i0 = i1 - 1;

    const float d0 = arcLength_[i0];
    const float d1 = arcLength_[i1];
    const float u0 = static_cast<float>(i0) / (arcLength_.size() - 1);
    const float u1 = static_cast<float>(i1) / (arcLength_.size() - 1);

    const float denom = (d1 - d0);
    if (std::fabs(denom) < 1e-6f) return u0; // 同一点に潰れているなど

    const float f = (m - d0) / denom;
    return u0 + (u1 - u0) * f;
}

void Spline::InvalidateArcLength_() noexcept {
    arcLength_.clear();
    totalLength_ = 0.0f;
    arcBuilt_ = false;
}