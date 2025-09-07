#pragma once
#include "BaseObject/BaseObject.h"
#include "Vector3.h"

class GlobalParameter;
class BaseStation;
class NpcFireController;

class BoundaryBreaker : public BaseObject {
public:
    BoundaryBreaker();
    ~BoundaryBreaker() override;

    // 既存の初期化・更新
    void Init() override;
    void Update() override;

    // 既存の Move() はロジック変更禁止：既存定義をそのまま利用
    void Move();

    // 射撃（状態/回数/クールダウンのガードを内部に追加）
    void Shoot();

    // 参照注入（const 許容に変更）
    void SetRivalStation(const BaseStation* s) noexcept { pRivalStation_ = s; }

    // インストーラ互換 API
    void SetAnchorPoint(const Vector3& p) noexcept { anchorPosition_ = p; }
    void SetPhase(float ph) noexcept { phase_ = ph; }
    void SetRadius(float r) noexcept { turningRadius_ = r; }

    // 参考：外部から読みたい時用
    float GetTurningRadius() const noexcept { return turningRadius_; }

protected:
    // パラメータバインド（既存と統合OK）
    virtual void BindParms();

private:
    // ===== サイクル制御 =====
    enum class State { Move, Attack };

    void BeginMoveCycle();         // 攻撃→移動へ
    void BeginAttackCycle();       // 移動→攻撃へ
    void RecomputeShotsPerCycle(); // shotsPerCycle_ = round(100 - breakerDamage_)

private:
    // ===== 参照 =====
    GlobalParameter* globalParam_ = nullptr;
    const BaseStation* pRivalStation_ = nullptr; // ★ const 許容
    std::unique_ptr<NpcFireController> fireController_ = nullptr;

    // ===== パラメータ（GlobalParameterで編集可） =====
    std::string groupName_ = "BoundaryBreaker";
    float       speed_ = 20.0f;     // m/s（Move() が参照していればそのまま）
    float       shootInterval_ = 2.0f;     // s：連射間隔
    float       moveDuration_ = 3.0f;      // s：移動フェーズの長さ
    float       breakerDamage_ = 50.0f;     // 1発のダメージ（BoundaryBreakerBullet と合わせる）
    float       turningRadius_ = 15.0f;     // ← SetRadius() で設定

    // ===== ランタイム：サイクル =====
    State state_ = State::Move;
    float shootCooldown_ = 0.0f;  // 射撃クールダウン
    float moveTimer_ = 0.0f;  // 残り移動時間
    int   shotsPerCycle_ = 1;     // round(100 - breakerDamage_) をクランプ
    int   shotsFired_ = 0;     // 攻撃フェーズで撃った数

    // ===== ランタイム：移動（既存 Move() が参照するメンバ）=====
    float  angle_ = 0.0f;               // 現在角度（例：angle_ += speed_*dt で増加）
    float  phase_ = 0.0f;               // 初期位相（スポーンごとにずらす等）
    Vector3 anchorPosition_{ 0,0,0 };     // 周回の中心（例：拠点位置など）
};
