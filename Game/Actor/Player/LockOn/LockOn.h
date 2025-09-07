#pragma once
#include "2d/Sprite.h"
#include "3d/ViewProjection.h"
#include "Details/Faction.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Vector3.h"

#include <memory>
#include <optional>
#include <variant>
#include <vector>

// 前方宣言
class EnemyNPC;
class BoundaryBreaker;

class LockOn {
public:
    using LockOnVariant = std::variant<EnemyNPC*, BoundaryBreaker*>;

public:
    LockOn()  = default;
    ~LockOn() = default;

    void Init();
    void Update(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction);
    void Draw();

    void OnObjectDestroyed(const LockOnVariant& obj);

    // ホーミングミサイル制御
    void SetHomingMissileActive(bool isActive) { isHomingMissileActive_ = isActive; }
    bool IsHomingMissileActive() const { return isHomingMissileActive_; }

    // editor
    void AdjustParam();
    void BindParams();

private:
    // ヘルパー関数
    Vector3 GetPosition(const LockOnVariant& target) const;
    bool IsLockable(const LockOnVariant& target, FactionType playerFaction) const;
    /*  bool IsDead(const LockOnVariant& target) const;
    FactionType GetFaction(const LockOnVariant& target) const;*/

    // 自動検索関連
    void AutoSearchTarget(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction);

    // 手動切り替え関連
    void HandleTargetSwitching(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction);

    // ターゲット検索・管理
    std::vector<LockOnVariant> GetValidTargets(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction) const;
    bool IsTargetRange(const LockOnVariant& target, const ViewProjection& viewProjection, Vector3& positionView) const;
    void SortTargetsByDistance(std::vector<std::pair<float, LockOnVariant>>& validTargets) const;
    void SortTargetsByAngle(std::vector<std::pair<float, LockOnVariant>>& validTargets) const;

    // UI更新
    void UpdateUI(const ViewProjection& viewProjection);
    void LerpTimeIncrement(float incrementTime);

private:
    // global parameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "LockOn";

    // Sprite
    std::unique_ptr<Sprite> lockOnMark_;

    // ターゲット
    std::optional<LockOnVariant> currentTarget_;
    std::vector<LockOnVariant> validTargets_;
    size_t currentTargetIndex_ = 0;

    // Sprite Param
    Vector2 prePos_;
    Vector2 spriteScale_;
    Vector2 lockOnMarkPos_;
    float spriteRotation_;

    // 自動検索設定
    bool autoSearchEnabled_   = true;
    float autoSearchInterval_ = 0.1f;
    float autoSearchTimer_    = 0.0f;

    // 切り替え
    float lerpTime_;
    bool prevSwitchInput_ = false;
    float targetChangeSpeed_;

    // ホーミングミサイル制御
    bool isHomingMissileActive_ = false;

    // パラメータ
    float minDistance_; //< 最小距離
    float maxDistance_; //< 最大距離
    float angleRange_; //< 角度範囲

    // 切り替えモード
    enum class SwitchMode {
        Distance, // 距離順
        Angle // 角度順
    };
    SwitchMode switchMode_ = SwitchMode::Distance;

public:
    bool ExistTarget() const { return currentTarget_.has_value(); }
    Vector3 GetTargetPosition() const;

    const LockOnVariant* GetCurrentTarget() const;
};

  