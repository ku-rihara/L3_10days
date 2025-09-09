#pragma once

// engine
#include "Actor/Spline/Spline.h"

// c++
#include <string>
#include <vector>
#include <filesystem>

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
private:
    /* ========================================================================
    /*  private structs
    /* ===================================================================== */
    struct RouteUnit {
        std::unique_ptr<Spline> spline = nullptr;
        Vector3 basePosition{};
        Vector3 preBasePosition{};
        Vector3 baseSize{};

        void Init(const std::string& filePath);
        void Update();
        void DrawDebug(const class ViewProjection& vp) const;
    };

public:
    /* ========================================================================
    /*  public funcs
    /* ===================================================================== */
    ~Route();                      // 生成した Spline の破棄
    void Init();                   // 各ルートの初期化
    void Update();
    void DrawDebug(const class ViewProjection& vp) const;

private:
    /* ========================================================================
    /*  private helpers
    /* ===================================================================== */
    // RouteType → "Route_<EnumName>.json" というファイル名を作る
    std::string MakeRouteFileName(RouteType t) const;
    std::string MakeRouteFilePath(RouteType t) const;

private:
    /* ========================================================================
    /*  private variable
    /* ===================================================================== */
    std::vector<RouteUnit> units_;
    const std::string fileDirectory_ = "resources/GlobalParameter/GameActor/NpcRoute";
};