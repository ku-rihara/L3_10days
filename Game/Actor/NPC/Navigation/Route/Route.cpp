//============================= Route.cpp =============================//
#include "Route.h"
#include "3d/ViewProjection.h"

#include <cassert>
#include <cmath>
#include <random>
#include <algorithm>

//============================= Unit ===================================//

void Route::RouteUnit::Load(const std::string& filePath) {
    spline = std::make_unique<Spline>();
    spline->Load(filePath);

    originalCps_.clear();
    if (spline) {
        const auto& cps = spline->GetControlPoints();
        originalCps_.assign(cps.begin(), cps.end());
    }
    preBasePosition = Vector3(99999, 99999, 99999); // 初回強制再適用
}

void Route::RouteUnit::Update() {
    if (!spline) return;

    if (preBasePosition != basePosition) {
        auto& cps = spline->GetControlPointsMutable();
        cps.resize(originalCps_.size());
        for (size_t i = 0; i < originalCps_.size(); ++i) { cps[i] = originalCps_[i] + basePosition; }
        preBasePosition = basePosition;
    }
}

void Route::RouteUnit::DrawDebug(const ViewProjection& vp) const { if (spline) spline->DebugDraw(vp); }

//============================= Route ===================================//

Route::Route(RouteType type) : type_(type) {}
Route::Route() = default;
Route::~Route() = default;


void Route::Init(RouteType type, const std::string& baseDir) {
    type_ = type;
    LoadFromDirectory(baseDir);
}

std::string Route::EnumName(RouteType t) {
    switch (t) {
        case RouteType::AllyDifence: return "AllyDifence";
        case RouteType::AllyAttack:  return "AllyAttack";
        case RouteType::EnemyDirence:return "EnemyDirence";
        case RouteType::EnemyAttack: return "EnemyAttack";
        default: return "Unknown";
    }
}

std::vector<std::filesystem::path>
Route::FindFilesForType_(RouteType t, const std::string& dir) const {
    std::vector<std::filesystem::path> out;

    // 探索ディレクトリ = baseDir / EnumName(t)
    const std::filesystem::path typeDir = std::filesystem::path(dir) / EnumName(t);

    // サブディレクトリが存在しなければ空で返す
    if (!std::filesystem::exists(typeDir) || !std::filesystem::is_directory(typeDir)) {
        return out;
    }

    const std::string ext = ".json";

    // typeDir 直下のファイルのみ列挙（再帰しない）
    for (auto& p : std::filesystem::directory_iterator(typeDir)) {
        if (!p.is_regular_file()) continue;
        const std::string name = p.path().filename().string();

        // 拡張子が .json のファイルだけを対象にする
        if (name.size() >= ext.size() &&
            name.rfind(ext) == name.size() - ext.size()) {
            out.push_back(p.path());
        }
    }

    std::sort(out.begin(), out.end());
    return out;
}


void Route::LoadFromDirectory(const std::string& baseDir) {
    variants_.clear();

    auto files = FindFilesForType_(type_, baseDir);
    for (auto& f : files) {
        RouteUnit u;
        u.Load(f.string());
        variants_.push_back(std::move(u));
    }
    activeIndex_ = variants_.empty() ? -1 : 0;
}

void Route::Update() { for (auto& v : variants_) v.Update(); }
void Route::DrawDebug(const ViewProjection& vp) const { for (const auto& v : variants_) v.DrawDebug(vp); }

void Route::SetBaseOffset(const Vector3& p) {
    for (auto& v : variants_) {
        v.basePosition = p; // Update() で original + basePosition に再適用される
    }
}

void Route::SwitchVariantKeepU(float /*u*/) {
    if (variants_.empty()) {
        // 何もない
        return;
    }
    if (variants_.size() == 1) {
        // 切り替え先が無いのでそのまま
        return;
    }

    // 別のインデックスを等確率で選ぶ
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, static_cast<int>(variants_.size()) - 1);

    int newIdx = activeIndex_;
    // 同じインデックスは避ける
    for (int guard = 0; guard < 8 && newIdx == activeIndex_; ++guard) {
        newIdx = dist(rng);
    }
    if (newIdx == activeIndex_) {
        // まれに同一を引き続けた場合は手動で次を選ぶ
        newIdx = (activeIndex_ + 1) % static_cast<int>(variants_.size());
    }

    // アクティブを切り替え（u は維持：SplineFollower 側の u_ をそのまま使う）
    activeIndex_ = newIdx;
}

void Route::ChooseRandomVariant(std::optional<uint32_t> seed) {
    if (variants_.empty()) {
        activeIndex_ = -1;
        return;
    }
    std::mt19937 rng(seed.value_or(std::random_device{}()));
    std::uniform_int_distribution<int> dist(0, static_cast<int>(variants_.size()) - 1);
    activeIndex_ = dist(rng);
}

Vector3 Route::Sample(float u) const {
    if (variants_.empty() || activeIndex_ < 0 || activeIndex_ >= static_cast<int>(variants_.size())) return {};
    const RouteUnit& unit = variants_[activeIndex_];
    if (!unit.spline) return {};
    return unit.spline->Sample(u); // ← Spline に合わせる
}

void Route::SetActiveIndex(int idx) {
    if (variants_.empty()) {
        activeIndex_ = -1;
        return;
    }
    idx = std::clamp(idx, 0, static_cast<int>(variants_.size()) - 1);
    activeIndex_ = idx;
}
