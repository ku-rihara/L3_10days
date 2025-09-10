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

    hitSprite_.reset(Sprite::Create(HitUI, Vector2::ZeroVector(), Vector4::kWHITE()));
    destroySprite_.reset(Sprite::Create(DestroyedUIHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
}

void EnemyInfoUI::Update() {
    hitSprite_->SetPosition(Vector2(640.0f, 320.0f));
    destroySprite_->SetPosition(Vector2(640.0f, 320.0f));
}

void EnemyInfoUI::Draw() {
}