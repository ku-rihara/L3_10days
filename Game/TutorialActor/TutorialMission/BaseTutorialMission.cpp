#include "BaseTutorialMission.h"
#include "Actor/player/Player.h"
#include "base/TextureManager.h"
#include "Frame/Frame.h"
#include <algorithm>
#include <imgui.h>

void BaseTutorialMission::Init(const std::string& name, const std::string& spriteName) {
    groupName_ = name;
    // GlobalParameterの初期化
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // スプライトセット
    int handle = TextureManager::GetInstance()->LoadTexture(filePath_ + spriteName + ".png");
    sprite_.reset(Sprite::Create(handle, Vector2::ZeroVector(), Vector4::kWHITE()));
    sprite_->anchorPoint_ = (Vector2(0.5f, 0.5f));

    // ゲージの初期化
    InitializeGauge();

    // 時間リセット
    currentTime_ = 0.0f;
    totalTime_   = 0.0f;
    progress_    = 0.0f;

    // ステータス初期化
    status_           = MissionStatus::NOT_STARTED;
    isAbleEnemySpawn_ = false;

    // スケール演出初期化
    scaleAnimState_ = ScaleAnimationState::NONE;
    tempScale_      = Vector2{0.0f, 0.0f};

    // イージング初期化
    EasingInit();
}

void BaseTutorialMission::Update() {

    sprite_->SetPosition(spritePos_);
    sprite_->SetScale(tempScale_);

    okSprite_->SetPosition(spritePos_);
    okSprite_->SetScale(tempScale_);

    gaugeFillSprite_->SetScale(Vector2(1.0f, tempScale_.y));
    gaugeBackSprite_->SetScale(Vector2(1.0f, tempScale_.y));

    // スケール演出の更新
    UpdateScaleAnimation();

    // ゲージの更新
    UpdateGauge();

    if (status_ == MissionStatus::IN_PROGRESS) {
        totalTime_ += Frame::DeltaTime();

        OnMissionUpdate();
    }
}

void BaseTutorialMission::EndUpdate() {
    // 派生クラスでオーバーライド可能
}

void BaseTutorialMission::SpriteDraw() {

    if (status_ == MissionStatus::COMPLETED || scaleAnimState_ == ScaleAnimationState::CLOSING) {
        okSprite_->Draw();
    } else {
        // メインスプライトの描画
        if (sprite_ && (scaleAnimState_ != ScaleAnimationState::NONE)) {
            sprite_->Draw();
        }
    }

    // ゲージの描画
    DrawGauge();
}

void BaseTutorialMission::StartMission() {
    if (status_ == MissionStatus::NOT_STARTED) {
        status_      = MissionStatus::IN_PROGRESS;
        currentTime_ = 0.0f;
        totalTime_   = 0.0f;
        progress_    = 0.0f;
        showGauge_   = true;

        // スケール演出開始
        StartScaleAppearAnimation();
        OnMissionStart();
    }
}

void BaseTutorialMission::CompleteMission() {
    if (status_ == MissionStatus::IN_PROGRESS) {
        status_    = MissionStatus::COMPLETED;
        progress_  = 1.0f;
        showGauge_ = false;

        // スケール演出終了を開始
        StartScaleCloseAnimation();

        // 派生クラスの完了処理
        OnMissionComplete();
    }
}
bool BaseTutorialMission::ProcessStep(float limitTime, bool enableEnemySpawn) {
    currentTime_ += Frame::DeltaTime();

    if (currentTime_ >= limitTime) {
        currentTime_ = 0.0f;
        if (enableEnemySpawn) {
            isAbleEnemySpawn_ = true;
        }
        return true;
    }
    return false;
}

void BaseTutorialMission::StartScaleAppearAnimation() {
    if (appearEase_.xScale && appearEase_.yScale) {
        scaleAnimState_ = ScaleAnimationState::APPEARING;

        // アピア演出をリセットして開始
        appearEase_.xScale->Reset();
        appearEase_.yScale->Reset();

        // 演出終了時のコールバック設定
        appearEase_.yScale->SetOnFinishCallback([this]() {
            if (scaleAnimState_ == ScaleAnimationState::APPEARING) {
                scaleAnimState_ = ScaleAnimationState::VISIBLE;
            }
        });
    }
}

void BaseTutorialMission::StartScaleCloseAnimation() {
    if (closeEase_.yScale) {
        scaleAnimState_ = ScaleAnimationState::CLOSING;

        // クローズ演出をリセットして開始

        closeEase_.yScale->Reset();

        // 演出終出時のコールバック設定
        closeEase_.yScale->SetOnFinishCallback([this]() {
            if (scaleAnimState_ == ScaleAnimationState::CLOSING) {
                scaleAnimState_ = ScaleAnimationState::NONE;
            }
        });
    }
}

void BaseTutorialMission::UpdateScaleAnimation() {
    float deltaTime = Frame::DeltaTime();

    switch (scaleAnimState_) {
    case ScaleAnimationState::APPEARING:
        if (appearEase_.xScale && appearEase_.yScale) {
            appearEase_.xScale->Update(deltaTime);
            appearEase_.yScale->Update(deltaTime);
            tempScale_ = appearEase_.tempScale_;
        }
        break;

    case ScaleAnimationState::CLOSING:
        if (closeEase_.yScale) {
            closeEase_.yScale->Update(deltaTime);
            tempScale_.y = closeEase_.tempScale_.y;
            tempScale_.x = 1.0f;
        }
        break;

    case ScaleAnimationState::VISIBLE:
    case ScaleAnimationState::NONE:
    default:
        // 何もしない
        break;
    }
}

bool BaseTutorialMission::IsScaleAnimationFinished() const {
    switch (scaleAnimState_) {
    case ScaleAnimationState::APPEARING:
        // アピアアニメーションの終了判定
        return (appearEase_.xScale && appearEase_.xScale->IsFinished()) && (appearEase_.yScale && appearEase_.yScale->IsFinished());

    case ScaleAnimationState::CLOSING:
        // クローズアニメーションの終了判定
        return (closeEase_.yScale && closeEase_.yScale->IsFinished());

    case ScaleAnimationState::VISIBLE:

        return true;

    case ScaleAnimationState::NONE:

        return true;

    default:
        return false;
    }
}

void BaseTutorialMission::InitializeGauge() {
    // ゲージ背景スプライトの作成
    int backHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + "MissionGauge.png");
    gaugeBackSprite_.reset(Sprite::Create(backHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    gaugeBackSprite_->anchorPoint_ = Vector2(0.0f, 0.5f);

    // ゲージ前景スプライトの作成
    int fillHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + "MissionGauge.png");
    gaugeFillSprite_.reset(Sprite::Create(fillHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    gaugeFillSprite_->anchorPoint_ = Vector2(0.0f, 0.5f);

    int okHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + "OK.png");
    okSprite_.reset(Sprite::Create(okHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    okSprite_->anchorPoint_ = Vector2(0.5f, 0.5f);

    // ゲージの初期設定
    showGauge_ = false;
}

void BaseTutorialMission::UpdateGauge() {

    // ゲージ位置の更新
    gaugeBackSprite_->SetPosition(gaugePos_);

    Vector2 fillPos = gaugePos_;
    fillPos.x -= gaugeSize_.x * 0.5f; // 左端から開始
    gaugeFillSprite_->SetPosition(fillPos);

    // ゲージのスケールを進行度に応じて設定
    Vector2 fillScale = Vector2(progress_, 1.0f);
    gaugeFillSprite_->SetScale(fillScale);
}

void BaseTutorialMission::DrawGauge() {

    gaugeBackSprite_->Draw();
    gaugeFillSprite_->Draw();
}

void BaseTutorialMission::BindParams() {
    globalParameter_->Bind(groupName_, "spritePos", &spritePos_);
    globalParameter_->Bind(groupName_, "gaugePos", &gaugePos_);
    globalParameter_->Bind(groupName_, "gaugeSize", &gaugeSize_);
    globalParameter_->Bind(groupName_, "showGauge", &showGauge_);
}

void BaseTutorialMission::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragFloat2("spritePos", &spritePos_.x, 0.01f);
        ImGui::DragFloat2("gaugePos", &gaugePos_.x, 0.01f);
        ImGui::DragFloat2("gaugeSize", &gaugeSize_.x, 0.01f);
        ImGui::Checkbox("showGauge", &showGauge_);

        AdjustUniqueParam();

        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}

void BaseTutorialMission::EasingInit() {
    appearEase_.xScale = std::make_unique<Easing<float>>();
    appearEase_.yScale = std::make_unique<Easing<float>>();
    appearEase_.xScale->Init("MissionStartX", "MissionStartX.json");
    appearEase_.yScale->Init("MissionStartY", "MissionStartY.json");
    appearEase_.xScale->SetAdaptValue(&appearEase_.tempScale_.x);
    appearEase_.yScale->SetAdaptValue(&appearEase_.tempScale_.y);

    closeEase_.yScale = std::make_unique<Easing<float>>();
    closeEase_.yScale->Init("MissionCloseY", "MissionCloseY.json");
    closeEase_.yScale->SetAdaptValue(&closeEase_.tempScale_.y);
}