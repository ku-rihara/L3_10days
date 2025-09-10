#pragma once

#include "../Base/BaseStation.h"
#include "Actor/NPC/PlayerNPC.h"
#include "Actor/NPC/Pool/NpcPool.h"

class PlayerStation final : public BaseStation {
public:
    /// ===================================================
    ///  public func
    /// ===================================================
    PlayerStation();
    PlayerStation(const std::string& name = "UnnamedStation");
    ~PlayerStation() override;

    void Init() override;
    void Update() override;

    void SpawnNPC(const Vector3& spawnPos) override;



    void SpriteUpdate(const ViewProjection& viewPro);
    void DrawSprite();

private:
    /// ===================================================
    ///  private variable
    /// ===================================================
    NpcPool<PlayerNPC> pool_;

private:
    std::unique_ptr<Sprite> bossReticle_;
    float spriteRotation_;
    bool isDraw_ = false;
};