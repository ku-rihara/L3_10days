#include "EnemyNPC.h"

#include "Actor/NPC/Bullet/FireController/NpcFierController.h"

EnemyNPC::EnemyNPC() {
    // 調整用グループ名
    groupName_ = "EnemyNPC";
    // 派閥
    SetFaction(FactionType::Enemy);
}

EnemyNPC::~EnemyNPC() {
}

/// ===================================================
///  初期化
/// ===================================================
void EnemyNPC::Init() {
    if (isInitialized_) {
        return;
    }
    isInitialized_ = true;

    obj3d_.reset(Object3d::CreateModel("Enemy.obj")); // 味方npcだけどいったんモデルを代用
    BaseObject::Init();
    obj3d_->transform_.parent_ = &baseTransform_;

    NPC::Init();
    SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

    /// =======================↓追加↓===============================

    // ターゲットマネージャー追加
    targetManager_ = TargetManager::GetInstance();
    if (targetManager_) {
        // 自分自身をvariantとして登録
        LockOn::LockOnVariant selfVariant = this;
        myTargetId_                       = targetManager_->RegisterTarget(selfVariant);
    }
}

/// ===================================================
///  更新
/// ===================================================
void EnemyNPC::Update() {
    NPC::Update();
}

/// =======================↓追加↓===============================

/// ===================================================
///  死んだタイミングとかでこれ呼びたい
/// ===================================================
void EnemyNPC::Finalize() {
    if (targetManager_ && myTargetId_ != INVALID_TARGET_ID) {
        targetManager_->UnregisterTarget(myTargetId_);
        myTargetId_ = INVALID_TARGET_ID;
    }
}