#pragma once

#include "2d/Sprite.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Vector3.h"
#include <memory>
#include <string>
#include <Vector2.h>

class Player;
class ViewProjection;
class PlayerReticle {
public:
    PlayerReticle()  = default;
    ~PlayerReticle() = default;

    // 初期化、更新、描画
    void Init();
    void Update(const Player* player, const ViewProjection* viewProjection);
    void Draw();

    // editor
    void AdjustParam();
    void BindParams();

private:
    Vector2 WorldToScreen(const Vector3& worldPos, const ViewProjection& viewProjection);

private:
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "PlayerReticle";

    // Sprite
    std::unique_ptr<Sprite> sprite_;

    // Pos
    Vector3 worldPos_;
    Vector2 screenPos_;

    // 前方の距離
    float forwardDistance_;

public:
    const Vector3& GetWorldPos() const { return worldPos_; }
};