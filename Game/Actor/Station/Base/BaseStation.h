#pragma once

// engine
#include "Collider/AABBCollider.h"

// game
#include "BaseObject/BaseObject.h"
#include "Details/Faction.h"
#include "Actor/NPC/Pool/NpcPool.h"
#include "utility/ParameterEditor/GlobalParameter.h"

// AI
#include "Actor/Station/UnitDirector/IUnitDirector.h"
#include "AI/Station/StationAI.h"

// targeting
#include "Actor/NPC/Bullet/Targeting.h"

// std
#include <string>
#include <vector>
#include <optional>

// fwd
class NPC;
class RouteCollection;
class Route;
class Spline;
enum class RouteType;
class FireController_;

class NpcBullet;

class BaseStation :
    public BaseObject,
    public ITargetProvider,
    public AABBCollider {
public:
    explicit BaseStation(const std::string& name = "UnnamedStation");
    ~BaseStation() override;

    // lifecycle
    void Init() override;
    void Update() override;
    void DrawDebug(const class ViewProjection&);
    virtual void ShowGui();

    // spawn
    virtual void StartupSpawn();

    // params IO
    virtual void BindParms();
    virtual void LoadData();
    virtual void SaveData();

    // accessors
    void SetRivalStation(BaseStation* rival) noexcept;
    BaseStation* GetRivalStation() const noexcept;
    void SetFaction(FactionType type) noexcept;
    FactionType GetFactionType() const noexcept;
    void SetRouteCollection(const RouteCollection* routeCollection) noexcept;
    const RouteCollection* GetRouteCollection() const noexcept;

    // === Route attach ===
    void SetRoute(Route* route, int unitIndex = 0) noexcept;
    void AttachRouteFromCollection(RouteCollection* coll, RouteType type, int unitIndex = 0) noexcept;
    void AttachRouteByFaction(RouteCollection* coll, int unitIndex = 0) noexcept;

    // AI accessors
    void    SetUnitDirector(IUnitDirector* d) noexcept { unitDirector_ = d; }
    float   GetHp()       const noexcept { return hp_; }
    float   GetMaxHp()    const noexcept { return maxLife_; }
    Vector3 GetPosition() const noexcept { return baseTransform_.translation_; }

    // NPC管理
    std::vector<NPC*> GetLiveNpcs() const;
    std::vector<const NpcBullet*> GetLiveNpcBullets() const;

    template <class Fn>
    void ForEachNpc(Fn&& fn) {
        for (auto& h : spawned_) {
            if (h && h->GetIsAlive()) fn(*h.get());
        }
    }

    // ===== ITargetProvider =====
    void CollectTargets(std::vector<const BaseObject*>& out) const override;

    // ===== Collider =====
    void OnCollisionEnter(BaseCollider* other) override;

protected:
    virtual void SpawnNPC(const Vector3& pos) = 0; // 派生でNPC生成
    void CleanupSpawnedList();                     // 死亡/無効NPCの掃除

private:
    // ステーション移動
    void MoveAlongRoute_(float dt) noexcept;

    // AIの希望人数に応じてNPCを防衛/攻撃に再割当て
    void ReassignRoles();

    // ルートサンプル（明示variant）
    Vector3 SampleVariantPos_(int variantIndex, float t01) const noexcept;

protected:
    // 調整用
    GlobalParameter* globalParam_{}; // Initで取得
    const std::string name_;         // 調整項目グループ名
    const std::string fileDirectory_ = "GameActor/Station";

    // パラメータ（保存対象）
    Vector3 initialPosition_{};
    float   maxLife_ = 100.0f;
    float   spawnInterbal_ = 5.0f; // 既存綴りに合わせ保持
    int     maxConcurrentUnits_ = 20;
    int     initialSpawnCount_ = 20;
    float   initialSpawnDistanceFromThis_ = 1000.0f;

    // ---- game state ----
    FactionType faction_{};
    float   hp_ = 100.0f;
    float   currentTime_ = 0.0f;
    Vector3 prevRoutePos_{};
    const RouteCollection* pRouteCollection_ = nullptr;
    std::vector<NpcHandle> spawned_;
    BaseStation* pRivalStation_ = nullptr; // ライバル拠点

    // === AI関連 ===
    IUnitDirector* unitDirector_ = nullptr;
    StationAI       ai_;
    StationAiConfig aiCfg_;
    float           homeThreatDebug_ = 0.0f; // 0..1 自陣圧デバッグ入力

    // 再計画タイマ
    float planTimer_ = 0.0f;

    // === ルート走行 ===
    bool   useRoute_          = true;   // true: スプライン周回
    float  routeLapTime_      = 50.0f;  // 一周時間[sec]
    float  routePhase01_      = 0.0f;   // 0..1 位相オフセット
    Route* route_             = nullptr; // 接続中ルート
    int    routeUnitIndex_    = 0;      // 未使用でも互換で保持
    float  routeT01_          = 0.0f;   // 0..1

    // === variant 固定（Outer） ===
    int    routeActiveIndex_  = -1;     // 使用するvariant（Outer想定）

    // === 1→0 の滑らか折り返し ===
    bool   smoothWrap_              = true;  // 折り返し補間を有効化
    float  wrapBlendDurationSec_    = 0.6f;  // 折り返し補間時間[sec]
    bool   wrapping_                = false; // 補間中
    float  wrapTimer_               = 0.0f;
    Vector3 wrapFromPos_{}, wrapToPos_{};

    bool  routeShuffleDuringPlay_ = false;
    float routeShuffleInterval_   = 15.0f;
    float routeShuffleTimer_      = 0.0f;
    float moveSpeed_ = 1.0f;
    float traveledDist_ = 0.0f;

    bool routeInitialized_ = false;

};