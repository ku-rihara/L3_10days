#include "EnemyInfoUI.h"

/// engine
#include "audio/Audio.h"
#include "base/TextureManager.h"
#include "Frame/Frame.h"

EnemyInfoUI::EnemyInfoUI()  = default;
EnemyInfoUI::~EnemyInfoUI() = default;

EnemyInfoUI* EnemyInfoUI::GetInstance() {
    static EnemyInfoUI instance;
    return &instance;
}

void EnemyInfoUI::Init() {

    int DestroyedUIHandle = TextureManager::GetInstance()->LoadTexture("Resources/Texture/UI/DestroyedUI.png");
    int HitUI             = TextureManager::GetInstance()->LoadTexture("Resources/Texture/UI/HitUI.png");
    audio_                = Audio::GetInstance();

    audioHandle_ = audio_->LoadWave("Resources/Sound/SE/BulletHit.wav");

    hitSprite_.reset(Sprite::Create(HitUI, Vector2::ZeroVector(), Vector4::kWHITE()));
    hitSprite_->anchorPoint_ = Vector2(0.5f, 0.5f);
    destroySprite_.reset(Sprite::Create(DestroyedUIHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    destroySprite_->anchorPoint_ = Vector2(0.5f, 0.5f);
    destroySprite_->SetColor(Vector3(1.0f, 0.0f, 0.0f));
}

void EnemyInfoUI::Update() {
    hitSprite_->SetPosition(Vector2(640.0f, 120.0f));
    destroySprite_->SetPosition(Vector2(640.0f, 220.0f));

    // αを減衰させる
    if (isHitDraw_) {
        float deltaTime = Frame::DeltaTime();
        hitAlpha_ -= deltaTime * 0.5f;
        if (hitAlpha_ <= 0.0f) {
            hitAlpha_  = 0.0f;
            isHitDraw_ = false;
        }
    }

    // αを減衰させる
    if (isDestroyDraw_) {
        float deltaTime = Frame::DeltaTime();
        destroyAlpha_ -= deltaTime * 0.5f;
        if (destroyAlpha_ <= 0.0f) {
            destroyAlpha_  = 0.0f;
            isDestroyDraw_ = false;
        }
    }
}

void EnemyInfoUI::SetHit() {
    isHitDraw_ = true;
    hitAlpha_  = 1.0f;
    audio_->PlayWave(audioHandle_, 1.0f);
}

void EnemyInfoUI::SetDestroy() {
    isDestroyDraw_ = true;
    destroyAlpha_  = 1.0f;
  
}

void EnemyInfoUI::Draw() {
    if (isHitDraw_) {
        hitSprite_->SetAlpha(hitAlpha_);
        hitSprite_->Draw();
    }

    if (isDestroyDraw_) {
        destroySprite_->SetAlpha(destroyAlpha_);
        destroySprite_->Draw();
    }
}