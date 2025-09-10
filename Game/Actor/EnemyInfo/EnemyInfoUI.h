#pragma once

/// engine
#include "2d/Sprite.h"
#include "utility/ParticleEditor/ParticleEmitter.h"
#include "Vector3.h"

/// std
#include <array>
#include <audio/Audio.h>
#include <memory>

class EnemyInfoUI final {
    EnemyInfoUI();
    ~EnemyInfoUI();

public:
    static EnemyInfoUI* GetInstance();

    void Init();
    void Update();
    void Draw();

    void SetHit();
    void SetDestroy();

private:
    std::unique_ptr<Sprite> hitSprite_;
    std::unique_ptr<Sprite> destroySprite_;

    bool isHitDraw_;
    int audioHandle_;
    float coolTime_;

    bool isDestroyDraw_;
    float destroyAlpha_;
    float coolTimeDestroy_;

    float hitAlpha_;

    Audio* audio_;
};
