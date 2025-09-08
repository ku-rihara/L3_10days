#pragma once
#include "2d/Sprite.h"
#include "BasePlayerUI.h"
#include "Easing/Easing.h"
#include <array>
#include <memory>
#include <string>

enum class LifeStep {
    FULL,
    MIDDLE,
    DAUNGER,
};

class PlayerLifeUI : public BasePlayerUI {
public:
    PlayerLifeUI()           = default;
    ~PlayerLifeUI() override = default;

    // init,update,draw
    void Init(const std::string& fileName) override;
    void Update() override;
    void Draw() override;

    // editor
    void AdjustParam() override;
    void BindParams() override;
    void AdjustUniqueParam() override;

    void UpdateLifeStep();

    void EasingSet();

private:
    std::array<float, 3> lifeStepPar_;
    std::array<Vector3, 3> colorByLifeStep_;
    LifeStep lifeStep_ = LifeStep::FULL;
    float alpha_       = 1.0f;
    Vector2 tempScale_;
    std::unique_ptr<Easing<float>> alphaEase_;
    std::unique_ptr<Easing<Vector2>> scaleEase_;

private:
    std::string ToString(const size_t& stepIndex);
};