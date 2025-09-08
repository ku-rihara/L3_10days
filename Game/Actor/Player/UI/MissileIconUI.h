#pragma once
#include "2d/Sprite.h"
#include "BasePlayerUI.h"
#include "Easing/Easing.h"
#include <array>
#include <memory>
#include <string>

class MissileIconUI : public BasePlayerUI {
public:
    MissileIconUI()           = default;
    ~MissileIconUI() override = default;

    // init,update,draw
    void Init(const std::string& fileName, const size_t& num = 0) override;
    void Update() override;
    void Draw() override;

    // editor
    void AdjustParam() override;
    void BindParams() override;
    void AdjustUniqueParam() override;

    void UpdateLifeStep();
    void UpdateAbleShot();

    void EasingSet();
    void CollingStart();

private:
    float redLineAlpha_;
    float endAlpha_;
    Vector2 tempScale_;
    Vector2 redLineScale_;
    std::unique_ptr<Easing<Vector2>> scaleEase_;
    std::unique_ptr<Sprite> redLineSprite_;
    bool isColling_;
    bool isCollEndEase_;
    size_t index_;
};