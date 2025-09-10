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

    void Init() override;
    void Update() override;

    // 挙動
    void Move();
    void Shoot();

    // 参照注入
    void SetRivalStation(const BaseStation* s) noexcept { pRivalStation_ = s; }

    // インストーラ互換 API
    void SetAnchorPoint(const Vector3& p) noexcept { anchorPosition_ = p; }
    void SetPhase(float ph) noexcept { phase_ = ph; }

    float GetTurningRadius() const noexcept { return turningRadius_; }

protected:
    virtual void BindParms();

private:
    // ===== 参照 =====
    GlobalParameter* globalParam_ = nullptr;
    const BaseStation* pRivalStation_ = nullptr;
    std::unique_ptr<NpcFireController> fireController_ = nullptr;

    // ===== パラメータ（GlobalParameterで編集可） =====
    std::string groupName_ = "BoundaryBreaker";

    // 速度まわり
    float speed_ = 40.0f;               // 線速度[m/s]
    float maxAngularSpeedDeg_ = 120.0f; // 角速度上限[deg/s]
    bool  useBaseRadiusForSpeed_ = true;// 角速度計算にベース半径を使う

    // 射撃
    float shootInterval_ = 5.0f;        // 連射間隔[s]

    // 周回ジオメトリ
    float turningRadius_ = 1000.0f;       // ベース半径
    float phase_ = 0.0f;                // 初期位相

    // ふわふわ（上下）＆呼吸（半径）
    float floatAmplitude_   = 30.0f;     // Y 振幅
    float floatFreq_        = 1.5f;     // Y 周波数
    float radiusWobbleAmp_  = 500.0f;    // 半径振幅
    float radiusWobbleFreq_ = 0.7f;     // 半径周波数

    // ===== ランタイム =====
    Vector3 anchorPosition_{0,0,0};
    float angle_ = 0.0f;                // 現在角度[rad]
    float floatTime_  = 0.0f;           // ふわふわ用時間
    float radiusTime_ = 0.0f;           // 呼吸半径用時間
    float shootCooldown_ = 0.0f;        // 射撃CD[s]
};