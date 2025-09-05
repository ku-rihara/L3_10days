#pragma once

#include"2d/Sprite.h"
#include <memory>

class PlayerReticle {
public:
    PlayerReticle()  = default;
    ~PlayerReticle() = default;

    // 初期化、更新、描画
    void Init();
    void Update();
    void Draw();

    //
private:
    // Sprite
    std::unique_ptr<Sprite> sprite_;
};