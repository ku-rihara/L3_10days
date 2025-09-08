#include "BoundaryBreaker.h"

#include "utility/ParameterEditor/GlobalParameter.h"
#include "Actor/Station/Base/BaseStation.h"

// ※ プロジェクトの実際のパス/綴りに合わせてください（NpcFierController など）
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"

#include "Frame/Frame.h"
#include <cmath>
#include <algorithm>

BoundaryBreaker::BoundaryBreaker() = default;

BoundaryBreaker::~BoundaryBreaker() = default;

// ==============================
// 初期化
// ==============================
void BoundaryBreaker::Init() {
    // 既存の初期化処理（モデル作成や親子付け等）がある場合はここで実行
    BaseObject::Init();
    obj3d_.reset(Object3d::CreateModel("Enemy.obj"));
    BaseObject::Init();

    obj3d_->transform_.parent_ = &baseTransform_;

    fireController_ = std::make_unique<NpcFireController>();
    fireController_->Init();

    // GlobalParameter のセットアップ
    globalParam_ = GlobalParameter::GetInstance();
    if (globalParam_) {
        globalParam_->CreateGroup(groupName_, true);
        globalParam_->ClearBindingsForGroup(groupName_);
        BindParms();
        globalParam_->SyncParamForGroup(groupName_);
    }

    // ---- 移動用の初期値補完（Move() ロジックは変更しない）----
    // アンカー未設定なら、対戦拠点 or 現在位置を初期中心にする
    if (anchorPosition_.x == 0.0f && anchorPosition_.y == 0.0f && anchorPosition_.z == 0.0f) {
        if (pRivalStation_) {
            anchorPosition_ = pRivalStation_->GetWorldPosition();
        } else {
            anchorPosition_ = GetWorldPosition();
        }
    }
    // angle_ / phase_ はデフォルト 0。必要なら外部から SetPhase() / SetAnchorPoint() / SetRadius() で与える

    // サイクル初期化
    RecomputeShotsPerCycle();
    BeginMoveCycle();
}

// ==============================
// 更新
// ==============================
void BoundaryBreaker::Update() {
    const float dt = Frame::DeltaTime();

    // GUI 等で damage 調整したら即反映されるように毎フレーム再計算
    RecomputeShotsPerCycle();
    fireController_->Tick();
    switch (state_) {
        case State::Move:
            // ★ Move() は既存の実装をそのまま使用（ここでは呼ぶだけ）
            Move();
            moveTimer_ -= dt;
            if (moveTimer_ <= 0.0f) {
                BeginAttackCycle();
            }
            break;

        case State::Attack:
            // 既存の Shoot() 名を流用しつつ、内部にガードを追加済み
            Shoot();
            if (shotsFired_ >= shotsPerCycle_) {
                BeginMoveCycle();
            }
            break;
    }

    // 既存の更新処理があれば最後に
    BaseObject::Update();
}

// ==============================
// パラメータバインド（必要に応じて追加）
// ==============================
void BoundaryBreaker::BindParms() {
    if (!globalParam_) return;
    globalParam_->Bind(groupName_, "speed", &speed_);
    globalParam_->Bind(groupName_, "shootInterval", &shootInterval_);
    globalParam_->Bind(groupName_, "moveDuration", &moveDuration_);
    globalParam_->Bind(groupName_, "breakerDamage", &breakerDamage_);
    globalParam_->Bind(groupName_, "turningRadius", &turningRadius_);
}

// ==============================
// 遷移ヘルパ
// ==============================
void BoundaryBreaker::BeginMoveCycle() {
    state_ = State::Move;
    moveTimer_ = moveDuration_;
    shotsFired_ = 0;
    shootCooldown_ = 0.0f;
}

void BoundaryBreaker::BeginAttackCycle() {
    state_ = State::Attack;
    shotsFired_ = 0;
    shootCooldown_ = 0.0f;
}

// (100 - damage) を整数化し、最低1発にクランプ
void BoundaryBreaker::RecomputeShotsPerCycle() {
    int n = static_cast<int>(std::lroundf(100.0f - breakerDamage_));
    if (n < 1) n = 1;
    shotsPerCycle_ = n;
}

//===================================================================*/
//				移動
//===================================================================*/
void BoundaryBreaker::Move() {
    const float dt = Frame::DeltaTime();

    angle_ += speed_ * dt;
    const float a = angle_ + phase_;

    Vector3 newPos;
    newPos.x = anchorPosition_.x + std::cos(a) * turningRadius_;
    newPos.z = anchorPosition_.z + std::sin(a) * turningRadius_;
    newPos.y = anchorPosition_.y;

    baseTransform_.translation_ = newPos;
}

// ==============================
// 射撃（状態・回数・クールダウンのガード追加版）
// ==============================
void BoundaryBreaker::Shoot() {
    if (state_ != State::Attack)       return; // 攻撃フェーズ以外は撃たない
    if (!fireController_)             return;
    if (!pRivalStation_)               return;
    if (shotsFired_ >= shotsPerCycle_) return;

    shootCooldown_ -= Frame::DeltaTime();
    if (shootCooldown_ > 0.0f) return;

    // ライバル拠点に向けて発射
    const Vector3 pos = GetWorldPosition();
    const Vector3 dir = (pRivalStation_->GetWorldPosition() - pos).Normalize();

    fireController_->SpawnBreaker(pos, dir);

    ++shotsFired_;
    shootCooldown_ = shootInterval_;
}
