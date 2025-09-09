#pragma once

// engine
#include "Actor/Spline/Spline.h"
#include "Vector3.h"

// c++
#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <random>
#include <optional>

/* ========================================================================
/*  enums
/* ===================================================================== */
enum class RouteType {
    AllyDifence,   //< 味方側のnpcの防御移動ルート
    AllyAttack,    //< 味方側のnpcの攻撃移動ルート
    EnemyDirence,  //< 敵側の防御移動ルート
    EnemyAttack    //< 敵側の攻撃移動ルート
};

class Route {
public:
    /* ========================================================================
    /*  struct: RouteUnit
    /* ===================================================================== */
    struct RouteUnit {
        std::unique_ptr<Spline> spline;
        Vector3 basePosition{};
        Vector3 preBasePosition{};
        Vector3 baseSize{1,1,1};
        std::vector<Vector3> originalCps_;   //< オフセット再適用用に元のCPを保持

        void Load(const std::string& filePath);
        void Update();
        void DrawDebug(const class ViewProjection& vp) const;
    };

public:
    Route() = default;
    ~Route();

    void Init(RouteType type, const std::string& dir);
    void Update();
    void DrawDebug(const class ViewProjection& vp) const;

    // ---- ランダム選択・切替API -----------------------------------------
    void ChooseRandomVariant(std::optional<uint32_t> seed = std::nullopt);
    void SwitchVariantKeepU(float u, std::optional<uint32_t> seed = std::nullopt);

    // ---- サンプリングAPI -----------------------------------------------
    Vector3 Sample(float u) const;

    // ---- accessor -------------------------------------------------------
    int  GetActiveIndex() const { return activeIndex_; }
    void SetActiveIndex(int idx);
    int  GetVariantCount() const { return static_cast<int>(variants_.size()); }
    RouteType GetType() const { return type_; }

    static std::string EnumName(RouteType t);

private:
    std::vector<std::filesystem::path> FindFilesForType_(RouteType t, const std::string& dir) const;

private:
    RouteType type_{RouteType::AllyDifence};
    std::vector<RouteUnit> variants_;
    int activeIndex_ = -1;
};