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

enum class RouteType {
    AllyDifence,
    AllyAttack,
    EnemyDirence,
    EnemyAttack
};

class Route {
public:
    struct RouteUnit {
        std::unique_ptr<Spline> spline;
        Vector3 basePosition{};
        Vector3 preBasePosition{};
        std::vector<Vector3> originalCps_;

        void Load(const std::string& filePath);
        void Update();
        void DrawDebug(const class ViewProjection& vp) const;
    };

public:
    Route(RouteType type);
    Route();
    ~Route();

    void LoadFromDirectory(const std::string& baseDir);
    void Init(RouteType type, const std::string& baseDir);
    void Update();
    void DrawDebug(const class ViewProjection& vp) const;

    void    ChooseRandomVariant(std::optional<uint32_t> seed = std::nullopt);

    // ★Vector3 に統一（Spline::Sample 型依存を排除）
    Vector3 Sample(float u) const;

    const RouteUnit* GetActiveUnit() const {
        if (activeIndex_ < 0 || activeIndex_ >= static_cast<int>(variants_.size())) return nullptr;
        return &variants_[activeIndex_];
    }

    int       GetActiveIndex() const { return activeIndex_; }
    void      SetActiveIndex(int idx);
    int       GetVariantCount() const { return static_cast<int>(variants_.size()); }
    RouteType GetType() const { return type_; }

    void SetBaseOffset(const Vector3& p);
       void SwitchVariantKeepU(float u);

    static std::string EnumName(RouteType t);

private:
    std::vector<std::filesystem::path> FindFilesForType_(RouteType t, const std::string& dir) const;

private:
    RouteType type_{ RouteType::AllyDifence };
    std::vector<RouteUnit> variants_;
    int activeIndex_ = -1;
};
