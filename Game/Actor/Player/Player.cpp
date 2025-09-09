#include "Player.h"
#include "Actor/Boundary/Boundary.h"
#include "Actor/GameCamera/GameCamera.h"
#include "Actor/NPC/Navigation/RectXZWithGatesConstraint.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "MathFunction.h"

// behavior
#include "Behavior/PlayerAcceleUnattended.h"
#include <cmath>
#include <imgui.h>
#include <numbers>

const std::vector<Hole>& Player::BoundaryHoleSource::GetHoles() const {
    static const std::vector<Hole> kEmpty;
    return boundary ? boundary->GetHoles() : kEmpty;
}

void Player::PartsInit() {
    // =========================後ろのWing=========================
    for (std::unique_ptr<PlayerBackWing>& backWing : backWings_) {
        backWing = std::make_unique<PlayerBackWing>();
    }
    // init
    backWings_[0]->Init(&baseTransform_, "BackWingLeft");
    backWings_[1]->Init(&baseTransform_, "BackWingRight");

    // =========================手前のWing=========================
    for (std::unique_ptr<PlayerFrontWing>& frontWing : frontWings_) {
        frontWing = std::make_unique<PlayerFrontWing>();
    }
    // init
    frontWings_[0]->Init(&baseTransform_, "LeftWing");
    frontWings_[1]->Init(&baseTransform_, "RightWing");

    // =========================中心のWing=========================
    backWingCenter_ = std::make_unique<PlayerBackWingCenter>();
    // init
    backWingCenter_->Init(&baseTransform_, "BackWingCenter");
}

void Player::UIInit() {
    // =========================LifeアイコンUI=========================
    lifeUI_ = std::make_unique<PlayerLifeUI>();
    lifeUI_->Init("PlayerLife");
    lifeUI_->SetPlayer(this);
    // =========================MissileアイコンUI=========================
    for (size_t i = 0; i < missileUIs_.size(); ++i) {
        missileUIs_[i] = std::make_unique<MissileIconUI>();
        missileUIs_[i]->Init("MissileUI", i);
        missileUIs_[i]->SetPlayer(this);
    }
    // =========================DMGテキストUI=========================
    dmgTextUI_ = std::make_unique<DMGTextUI>();
    dmgTextUI_->Init("DamageText");
    // =========================DMGParUI=========================
    dmgParUI_ = std::make_unique<PlayerDamageParUI>();
    dmgParUI_->Init("DamageParUI");
    dmgParUI_->SetPlayer(this);
}

void Player::Init() {

    // グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Player.obj"));
    obj3d_->transform_.rotateOder_ = RotateOder::Quaternion;

    // transform初期化
    baseTransform_.Init();
    baseTransform_.rotateOder_ = RotateOder::Quaternion;
    baseTransform_.quaternion_ = Quaternion::Identity();
    obj3d_->transform_.parent_ = &baseTransform_;

    // パーツ初期化
    PartsInit();
    // UI初期化
    UIInit();

    // レティクル
    reticle_ = std::make_unique<PlayerReticle>();
    reticle_->Init();

    // 弾初期化
    bulletShooter_ = std::make_unique<PlayerBulletShooter>();
    bulletShooter_->Init(&baseTransform_);

    // hpをSet
    hp_ = maxHp_;

    // moveConstraint
    Boundary* boundary   = Boundary::GetInstance();
    holeSource_.boundary = boundary;
    moveConstraint_      = std::make_unique<RectXZWithGatesConstraint>(&holeSource_, boundary->GetRectXZWorld(), 0.01f);

    // Speed Init
    SpeedInit();
    ChangeSpeedBehavior(std::make_unique<PlayerAccelUnattended>(this));
}

void Player::Update() {
    // 入力処理
    HandleInput();

    // スピード
    UpdateSpeedBehavior();
    speedBehavior_->Update();

    // 逆さ判定
    CheckIsUpsideDown();

    // 回転更新
    RotateUpdate();

    // 移動更新
    MoveUpdate();

    // パーツ更新
    PartsUpdate();

    // UI更新
    UIUpdate();

    // レティクル
    reticle_->Update(this, viewProjection_);

    // 弾丸システム更新
    if (bulletShooter_) {
        bulletShooter_->Update(this);
    }

    // トランスフォーム更新
    BaseObject::Update();
}

void Player::MoveUpdate() {

    float deltaTime = Frame::DeltaTime();

    ReboundByBoundary();

    // 前方速度
    Vector3 forwardVelocity = GetForwardVector() * speedParam_.currentForwardSpeed * deltaTime;
    velocity_               = forwardVelocity;

    // 跳ね返りの分を加えて適応
    baseTransform_.translation_ += velocity_ + reboundVelocity_;
}

void Player::PartsUpdate() {
    // 入力から目標回転を計算
    Vector3 backWingInputRotation       = Vector3::ZeroVector();
    Vector3 frontWingInputRotation      = Vector3::ZeroVector();
    Vector3 backCenterWingInputRotation = Vector3::ZeroVector();

    // =========================後ろのWing=========================
    // 上下入力をX回転に変換
    backWingInputRotation.x = -angleInput_.x * 0.2f;

    for (std::unique_ptr<PlayerBackWing>& backWing : backWings_) {
        backWing->SetInputRotation(backWingInputRotation);
        backWing->Update();
        backWing->SetBaseRotate(obj3d_->transform_.quaternion_.ToEuler());
    }

    // =========================手前のWing=========================
    // 上下入力をX回転に変換
    frontWingInputRotation.x = -angleInput_.x * 0.2f;

    for (std::unique_ptr<PlayerFrontWing>& frontWing : frontWings_) {
        frontWing->SetInputRotation(frontWingInputRotation);
        frontWing->Update();
        frontWing->SetBaseRotate(obj3d_->transform_.quaternion_.ToEuler());
    }

    // =========================後ろのWing=========================
    // 上下入力をX回転に変換
    backCenterWingInputRotation.y = -angleInput_.y * 0.2f;
    backWingCenter_->SetInputRotation(backCenterWingInputRotation);
    backWingCenter_->Update();
    backWingCenter_->SetBaseRotate(obj3d_->transform_.quaternion_.ToEuler());
}

void Player::UIUpdate() {
    // =========================LifeアイコンUI=========================
    lifeUI_->Update();

    // =========================MissileアイコンUI=========================
    for (size_t i = 0; i < missileUIs_.size(); ++i) {
        missileUIs_[i]->Update();
    }
    // =========================DMGテキストUI=========================
    dmgTextUI_->Update();
    // =========================DMGParUI=========================

    dmgParUI_->Update();
}

void Player::HandleInput() {
    Input* input = Input::GetInstance();

    // 入力値をリセット
    Vector2 stickL      = Vector2::ZeroVector();
    float pawInputValue = 0.0f;

    // ゲームパッドの入力
    stickL = Input::GetPadStick(0, 0);

    // キーボード入力
    if (stickL.Length() < 0.3f) {

        if (input->PushKey(DIK_W)) {
            stickL.y = 1.0f;
        }
        if (input->PushKey(DIK_S)) {
            stickL.y = -1.0f;
        }
        if (input->PushKey(DIK_A)) {
            stickL.x = -1.0f;
        }
        if (input->PushKey(DIK_D)) {
            stickL.x = 1.0f;
        }
    }

    // 逆さ時の自動操作
    if (invCorrectionParam_.isAutoRotate) {
        stickL.y = invCorrectionParam_.autoRotateDirection_;
    }

    // 跳ね返りによる自動操作
    if (reboundCorrectionParam_.isAutoRotate) {
        stickL.y = reboundCorrectionParam_.autoRotateDirection_;
    }

    // コントローラ処理
    if (stickL.Length() > 0.1f) {
        stickL = stickL.Normalize() * std::min(stickL.Length(), 1.0f);
    } else {
        stickL = {0.0f, 0.0f};
    }

    if (Input::IsPressPad(0, XINPUT_GAMEPAD_LEFT_SHOULDER) || input->PushKey(DIK_Q)) {
        pawInputValue = -1.0f;
    } else if (Input::IsPressPad(0, XINPUT_GAMEPAD_RIGHT_SHOULDER) || input->PushKey(DIK_E)) {
        pawInputValue = 1.0f;
    }

    // deltaTime
    float deltaTime = Frame::DeltaTime();

    // ピッチの回転スピードきめる
    float pitchSpeed = speedParam_.pitchSpeed;
    // リバウンドによる速度
    if (reboundCorrectionParam_.isAutoRotate) {
        pitchSpeed = reboundCorrectionParam_.autoRotateSpeed_;
        // 逆さによるリカバーの速度
    } else if (invCorrectionParam_.isAutoRotate) {
        pitchSpeed = invCorrectionParam_.autoRotateSpeed_;
    }

    // ピッチ
    angleInput_.x = -stickL.y * (pitchSpeed * deltaTime);

    // ヨー
    angleInput_.y = pawInputValue * (speedParam_.yawSpeed * deltaTime);

    // ロール
    rollInput_ = -stickL.x;
    targetRoll_ += rollInput_ * speedParam_.rollSpeed * deltaTime;

    // 制限
    if (fabs(rollInput_) < 0.001f) {
        currentMaxRoll_ = 0.0f;
    } else {
        angleInput_.y   = 0.0f;
        currentMaxRoll_ = ToRadian(rollRotateLimit_);
    }

    targetRoll_ = std::clamp(targetRoll_, -currentMaxRoll_, currentMaxRoll_);
}

void Player::RotateUpdate() {
    float deltaTime = Frame::DeltaTime();

    // ---- 通常のピッチ・ヨー回転処理 ----
    Vector3 targetAngularVelocity = angleInput_;
    const float damping           = 0.95f;
    if (angleInput_.Length() < 0.001f) {
        targetAngularVelocity = angularVelocity_ * damping;
    }
    angularVelocity_ = Lerp(angularVelocity_, targetAngularVelocity, 0.7f);

    Vector3 localRight = GetRightVector();
    Vector3 localUp    = GetUpVector();

    Quaternion pitchRotation = Quaternion::MakeRotateAxisAngle(localRight, angularVelocity_.x * deltaTime);
    Quaternion yawRotation   = Quaternion::MakeRotateAxisAngle(localUp, angularVelocity_.y * deltaTime);

    Quaternion deltaRotation = yawRotation * pitchRotation;
    targetRotation_          = deltaRotation * baseTransform_.quaternion_;
    targetRotation_          = targetRotation_.Normalize();

    // 補正処理
    CorrectionHorizon();

    // 適応
    baseTransform_.quaternion_ = Quaternion::Slerp(
        baseTransform_.quaternion_, targetRotation_, rotationSmoothness_);
    baseTransform_.quaternion_ = baseTransform_.quaternion_.Normalize();

    // ---- ロールを補間 ----
    currentRoll_ = Lerp(currentRoll_, targetRoll_, speedParam_.rollSpeed * deltaTime);

    float yawFromRoll = -sin(currentRoll_) * bankRate_ * deltaTime;
    if (fabs(yawFromRoll) > 0.0001f) {
        Quaternion yawFromRollRotation = Quaternion::MakeRotateAxisAngle(Vector3::ToUp(), yawFromRoll);
        baseTransform_.quaternion_     = yawFromRollRotation * baseTransform_.quaternion_;
        baseTransform_.quaternion_     = baseTransform_.quaternion_.Normalize();
    }

    // ---- obj3dのTransformのみロール適用 ----
    Quaternion visualRoll          = Quaternion::MakeRotateAxisAngle(Vector3::ToForward(), currentRoll_);
    obj3d_->transform_.quaternion_ = visualRoll;
    obj3d_->transform_.quaternion_ = obj3d_->transform_.quaternion_.Normalize();
}

void Player::CorrectionHorizon() {

    // 補正開始フラグ
    if (!invCorrectionParam_.isAutoRotate && isUpsideDown_ && angleInput_.Length() < 0.001f) {
        invCorrectionParam_.isAutoRotate = true;

        // 向きを決める
        float currentVelocityY = velocity_.y;

        // yの速度によって自動操作の向きを決める
        if (currentVelocityY > 0.0f) {
            invCorrectionParam_.autoRotateDirection_ = -1.0f;
        } else {
            invCorrectionParam_.autoRotateDirection_ = 1.0f;
        }
    }

    // 補正処理
    if (!invCorrectionParam_.isAutoRotate) {
        return;
    }

    // 自動操作終了
    if (upDot_ >= 0.5f) {
        invCorrectionParam_.isAutoRotate = false;
    }
}

bool Player::CheckIsRollMax() const {
    return fabsf(ToDegree(currentRoll_)) >= rollRotateLimit_ - rollRotateOffset_;
}

void Player::ReboundByBoundary() {

    // from,toを計算
    Vector3 from = baseTransform_.translation_;
    Vector3 to   = baseTransform_.translation_ + velocity_;

    // FilterMove
    if (moveConstraint_) {
        to = moveConstraint_->FilterMove(from, to);
    }

    // ブロック判定
    if (RectXZWithGatesConstraint* rectConstraint = dynamic_cast<RectXZWithGatesConstraint*>(moveConstraint_.get())) {
        isRebound_ = rectConstraint->WasBlocked();
    }

    // 跳ね返り処理
    if (isRebound_) {
        // Y だけ跳ね返す
        const float inVy   = velocity_.y;
        reboundVelocity_.y = -inVy * reboundPower_;

        float reboundVelocityY = reboundVelocity_.y;

        // ダメージ
        TakeDamageForBoundary();

        // 自動入力を開始
        if (reboundVelocityY > 0.0f) { // 上向き
            reboundCorrectionParam_.isAutoRotate         = true;
            reboundCorrectionParam_.autoRotateDirection_ = 1.0f;

        } else if (reboundVelocityY <= -0.0f) { // 下向き
            reboundCorrectionParam_.isAutoRotate         = true;
            reboundCorrectionParam_.autoRotateDirection_ = -1.0f;
        }

        // カメラシェイク
        pGameCamera_->PlayShake("PlayerHitBoundaryShake");
    }

    // 減衰処理
    if (std::abs(reboundVelocity_.y) > minReboundVelocity_) {
        reboundVelocity_.y *= reboundDecay_;
    } else {
        // 減衰おわり
        reboundVelocity_.y                   = 0.0f;
        reboundCorrectionParam_.isAutoRotate = false;
    }

    // upDotが1.0f付近になったら自動回転終わり
    if (upDot_ >= 0.95f) {
        reboundCorrectionParam_.isAutoRotate = false;
    }
}

void Player::CheckIsUpsideDown() {
    // 機体の上方向ベクトルを取得
    Matrix4x4 targetMatrix = MakeRotateMatrixQuaternion(targetRotation_);
    Vector3 targetUpVector = TransformNormal(Vector3::ToUp(), targetMatrix);

    // 機体の上方向とワールドの上方向の内積を計算
    Vector3 worldUp = Vector3::ToUp();
    upDot_          = Vector3::Dot(targetUpVector, worldUp);

    // 機体が逆さまかどうかを判定
    isUpsideDown_ = upDot_ < reverseDecisionValue_;
}

Vector3 Player::GetForwardVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrixQuaternion(baseTransform_.quaternion_);
    return TransformNormal(Vector3::ToForward(), rotationMatrix).Normalize();
}

Vector3 Player::GetRightVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrixQuaternion(baseTransform_.quaternion_);
    return TransformNormal(Vector3::ToRight(), rotationMatrix).Normalize();
}

Vector3 Player::GetUpVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrixQuaternion(baseTransform_.quaternion_);
    return TransformNormal(Vector3::ToUp(), rotationMatrix).Normalize();
}

void Player::ChangeSpeedBehavior(std::unique_ptr<BasePlayerSpeedBehavior> behavior) {
    if (speedBehavior_) {

        behavior->TransferStateFrom(speedBehavior_.get());
    }
    speedBehavior_ = std::move(behavior);
}

void Player::UpdateSpeedBehavior() {

    auto newBehavior = speedBehavior_->CheckForBehaviorChange();

    if (newBehavior) {

        ChangeSpeedBehavior(std::move(newBehavior));
    }
}

void Player::SpeedInit() {
    speedParam_.currentForwardSpeed = speedParam_.startForwardSpeed;
}

void Player::SpeedUpdate() {
    speedParam_.currentForwardSpeed = speedBehavior_->GetCurrentSpeed();
}

float Player::GetRollDegree() const {
    Vector3 currentEuler = obj3d_->transform_.quaternion_.ToEuler();
    return ToDegree(currentEuler.z);
}

void Player::ReticleDraw() {
    reticle_->Draw();
}

void Player::TakeDamageForBoundary() {
    hp_ -= damageValueByBoundary_;
}

void Player::BindParams() {
    globalParameter_->Bind(groupName_, "hp", &maxHp_);
    globalParameter_->Bind(groupName_, "forwardSpeed", &speedParam_.startForwardSpeed);
    globalParameter_->Bind(groupName_, "pitchSpeed", &speedParam_.pitchSpeed);
    globalParameter_->Bind(groupName_, "yawSpeed", &speedParam_.yawSpeed);
    globalParameter_->Bind(groupName_, "rollSpeed", &speedParam_.rollSpeed);
    globalParameter_->Bind(groupName_, "minForwardSpeed", &speedParam_.minForwardSpeed);
    globalParameter_->Bind(groupName_, "maxForwardSpeed", &speedParam_.maxForwardSpeed);
    globalParameter_->Bind(groupName_, "brakeForwardSpeed", &speedParam_.brakeForwardSpeed);
    globalParameter_->Bind(groupName_, "rotationSmoothness", &rotationSmoothness_);
    globalParameter_->Bind(groupName_, "rollRotateLimit", &rollRotateLimit_);
    globalParameter_->Bind(groupName_, "pitchBackTime", &pitchBackTime_);
    globalParameter_->Bind(groupName_, "rollBackTime", &rollBackTime_);
    globalParameter_->Bind(groupName_, "pitchReturnThreshold", &pitchReturnThreshold_);
    globalParameter_->Bind(groupName_, "reverseDecisionValue", &reverseDecisionValue_);
    globalParameter_->Bind(groupName_, "bankRate", &bankRate_);
    globalParameter_->Bind(groupName_, "reboundPower", &reboundPower_);
    globalParameter_->Bind(groupName_, "reboundDecay", &reboundDecay_);
    globalParameter_->Bind(groupName_, "minReboundVelocity", &minReboundVelocity_);
    globalParameter_->Bind(groupName_, "autoRotateSpeed", &reboundCorrectionParam_.autoRotateSpeed_);
    globalParameter_->Bind(groupName_, "autoRecoverSpeed", &invCorrectionParam_.autoRotateSpeed_);
    globalParameter_->Bind(groupName_, "rollRotateLimitOffset", &rollRotateOffset_);
    globalParameter_->Bind(groupName_, "damageValueByBoundary", &damageValueByBoundary_);
}

void Player::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragFloat("MaxHp", &maxHp_);
        ImGui::DragFloat("CurrentHP", &hp_);
        // EditParameter
        ImGui::Separator();
        ImGui::Text("Fighter Controls");
        ImGui::SeparatorText("Speed");
        ImGui::DragFloat("StartForward Speed", &speedParam_.startForwardSpeed, 0.01f);
        ImGui::DragFloat("minForward Speed", &speedParam_.minForwardSpeed, 0.01f);
        ImGui::DragFloat("maxForward Speed", &speedParam_.maxForwardSpeed, 0.01f);
        ImGui::DragFloat("brakeForward Speed", &speedParam_.brakeForwardSpeed, 0.01f);
        ImGui::DragFloat("Pitch Speed", &speedParam_.pitchSpeed, 0.01f);
        ImGui::DragFloat("Yaw Speed", &speedParam_.yawSpeed, 0.01f);
        ImGui::DragFloat("Roll Speed", &speedParam_.rollSpeed, 0.01f);

        ImGui::SeparatorText("AutoOperator");
        ImGui::DragFloat("AutoRotateReboundSpeed", &reboundCorrectionParam_.autoRotateSpeed_, 0.01f);
        ImGui::DragFloat("AutoRotateInvSpeed", &invCorrectionParam_.autoRotateSpeed_, 0.01f);

        ImGui::SeparatorText("rebound");
        ImGui::DragFloat("rebound Power", &reboundPower_, 0.01f);
        ImGui::DragFloat("rebound Decay", &reboundDecay_, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("min Rebound Velocity", &minReboundVelocity_, 0.01f, 0.0f, 10.0f);

        ImGui::SeparatorText("Roll");
        ImGui::DragFloat("rollBackTime", &rollBackTime_, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("rollRotateLimit", &rollRotateLimit_, 0.01f);
        ImGui::DragFloat("rollRotateOffset", &rollRotateOffset_, 0.01f);

        ImGui::SeparatorText("Damage");
        ImGui::DragFloat("damageValueByBoundary",&damageValueByBoundary_, 0.01f);

        ImGui::SeparatorText("etc");
        ImGui::DragFloat("rotationSmoothness", &rotationSmoothness_, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("pitchBackTime", &pitchBackTime_, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("pitchReturnThreshold", &pitchReturnThreshold_, 1.0f, 0.0f, 90.0f);
        ImGui::DragFloat("reverseDecisionValue", &reverseDecisionValue_, 0.01f, -1.0f, 0.0f);
        ImGui::DragFloat("bankRate", &bankRate_, 0.01f);

        // デバッグ
        ImGui::Separator();
        ImGui::Text("Debug Info");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
            baseTransform_.translation_.x,
            baseTransform_.translation_.y,
            baseTransform_.translation_.z);
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);
        ImGui::Text("Rebound Velocity: (%.2f, %.2f, %.2f)",
            reboundVelocity_.x, reboundVelocity_.y, reboundVelocity_.z);
        ImGui::Text("Collision Normal: (%.2f, %.2f, %.2f)",
            lastCollisionNormal_.x, lastCollisionNormal_.y, lastCollisionNormal_.z);

        // 機体姿勢のデバッグ
        ImGui::Separator();
        ImGui::Text("Aircraft Attitude");
        Vector3 euler = baseTransform_.quaternion_.ToEuler();
        ImGui::Text("Euler (deg): P=%.1f, Y=%.1f, R=%.1f",
            ToDegree(euler.x), ToDegree(euler.y), ToDegree(euler.z));

        ImGui::Text("upDot= %.1f", upDot_);
        if (isUpsideDown_) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: UPSIDE DOWN!");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "STATUS: Normal");
        }

        if (isRebound_) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "REBOUNDING!");
        }

        ImGui::Text("currentSpeed:%.3f", speedParam_.currentForwardSpeed);

        // セーブ・ロード
        ImGui::Separator();
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
    ImGui::SeparatorText("UIs");
    lifeUI_->AdjustParam();
    for (std::unique_ptr<MissileIconUI>& missileUI : missileUIs_) {
        missileUI->AdjustParam();
    }
    dmgTextUI_->AdjustParam();
    dmgParUI_->AdjustParam();

    ImGui::SeparatorText("Parts");
    // backWing
    for (std::unique_ptr<PlayerBackWing>& backWing : backWings_) {
        backWing->AdjustParam();
    }

    // frontWing
    for (std::unique_ptr<PlayerFrontWing>& frontWing : frontWings_) {
        frontWing->AdjustParam();
    }
    // backWingCenter
    if (backWingCenter_) {
        backWingCenter_->AdjustParam();
    }
    ImGui::SeparatorText("etc");
    // 弾
    if (bulletShooter_) {
        bulletShooter_->AdjustParam();
    }
    // レティクル
    if (reticle_) {
        reticle_->AdjustParam();
    }

#endif // _DEBUG
}

void Player::UIDraw() {
    lifeUI_->Draw();
    for (std::unique_ptr<MissileIconUI>& missileUI : missileUIs_) {
        missileUI->Draw();
    }
    dmgTextUI_->Draw();
    dmgParUI_->Draw();
}

void Player::SetGameCamera(GameCamera* camera) {
    pGameCamera_ = camera;
}

void Player::SetViewProjection(const ViewProjection* viewProjection) {
    viewProjection_ = viewProjection;
    bulletShooter_->SetViewProjection(viewProjection);
}

void Player::SetLockOn(LockOn* lockOn) {
    bulletShooter_->SetLockOn(lockOn);
}