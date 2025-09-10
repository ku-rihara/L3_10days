#pragma once

/// engine
#include "utility/ParticleEditor/ParticleEmitter.h"
#include"2d/Sprite.h"
#include "Vector3.h"

/// std
#include <array>
#include <memory>

class EnemyInfoUI final {
    EnemyInfoUI();
    ~EnemyInfoUI();

public:
    static EnemyInfoUI* GetInstance();

    void Init();
    void Update();
    void Draw();
 
private:
    std::unique_ptr<Sprite> hitSprite_;
    std::unique_ptr<Sprite> destroySprite_;
};
