#include "Route.h"
#include "3d/ViewProjection.h"
#include "Actor/Spline/Spline.h"

#include <cassert>
#include <cmath>
#include "../externals/magic_enum/magic_enum.hpp"

//============================= Unit ===================================//

/////////////////////////////////////////////////////////////////////////////////////////
//      初期化
/////////////////////////////////////////////////////////////////////////////////////////
void Route::RouteUnit::Init(const std::string& filePath) {
    assert(spline && "RouteUnit::Init: spline is null. allocate it before Init().");
    spline->Load(filePath);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      更新
/////////////////////////////////////////////////////////////////////////////////////////
void Route::RouteUnit::Update() {
    if (!spline) return;

    // 直前との差分（ほぼゼロなら何もしない）
    const Vector3 delta = basePosition - preBasePosition;
    if (std::fabs(delta.x) < 1e-6f &&
        std::fabs(delta.y) < 1e-6f &&
        std::fabs(delta.z) < 1e-6f) {
        return;
    }

    // 既存の全CPに差分だけ加算してオフセット
    auto& cps = spline->GetControlPointsMutable();
    for (auto& p : cps) {
        p += delta;
    }
    preBasePosition = basePosition;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      軌道描画
/////////////////////////////////////////////////////////////////////////////////////////
void Route::RouteUnit::DrawDebug(const ViewProjection& vp) const {
    if (spline) spline->DebugDraw(vp);
}

//============================= Route 本体 ===============================//

Route::~Route() =default;

/////////////////////////////////////////////////////////////////////////////////////////
//      private: ファイル名作成（例：Route_AllyDifence.json）
/////////////////////////////////////////////////////////////////////////////////////////
std::string Route::MakeRouteFileName(RouteType t) const {
    // enum_name は識別子文字列（AllyDifence 等）を返す
    const std::string enumStr = std::string(magic_enum::enum_name(t));
    // 必要ならここでスネークケース化や独自名に変換してもOK
    return std::string("Route_") + enumStr + ".json";
}

/////////////////////////////////////////////////////////////////////////////////////////
//      private: ディレクトリ + ファイル名 → フルパス
/////////////////////////////////////////////////////////////////////////////////////////
std::string Route::MakeRouteFilePath(RouteType t) const {
    std::filesystem::path p(fileDirectory_);
    p /= MakeRouteFileName(t);
    return p.string();
}

/////////////////////////////////////////////////////////////////////////////////////////
//      初期化
/////////////////////////////////////////////////////////////////////////////////////////
void Route::Init() {
    // すべての RouteType を列挙してロード（必要なものだけにしたければ適宜絞る）
    units_.reserve(magic_enum::enum_count<RouteType>());
    for (auto t : magic_enum::enum_values<RouteType>()) {
        RouteUnit unit{};
        unit.spline = std::make_unique<Spline>();
        unit.basePosition = {0, 0, 0};
        unit.preBasePosition = {0, 0, 0};
        unit.baseSize = {1, 1, 1};

        const std::string path = MakeRouteFilePath(t);
        unit.Init(path);

        units_.push_back(std::move(unit));
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//      更新
/////////////////////////////////////////////////////////////////////////////////////////
void Route::Update() {
    for (auto& u : units_) {
        u.Update();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//      描画
/////////////////////////////////////////////////////////////////////////////////////////
void Route::DrawDebug(const ViewProjection& vp) const {
    for (const auto& u : units_) {
        u.DrawDebug(vp);
    }
}