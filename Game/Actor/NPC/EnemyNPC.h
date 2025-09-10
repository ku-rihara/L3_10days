#pragma once

#include "Actor/Player/TargetManager/TargetManager.h"
#include "NPC.h"

class EnemyNPC final : public NPC {
public:
    /// ===================================================
    ///  public func
    /// ===================================================
    EnemyNPC();
    ~EnemyNPC() override;

    void Init() override;
    void Update() override;

    void Finalize();

    /// ===================================================
    /// collision methods
    /// ===================================================
     void OnCollisionEnter(BaseCollider* other) override;

private:
    /// =======================↓追加↓===============================
    TargetID myTargetId_          = INVALID_TARGET_ID;
    TargetManager* targetManager_ = nullptr;

public:
    /// =======================↓追加↓===============================
    TargetID GetTargetId() const { return myTargetId_; }
};