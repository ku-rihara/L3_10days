#pragma once
#include "2d/Sprite.h"
#include "BasePlayerUI.h"
#include "Easing/Easing.h"
#include <array>
#include <memory>
#include <string>

class DMGTextUI : public BasePlayerUI {
public:
    DMGTextUI()           = default;
    ~DMGTextUI() override = default;

    // init,update,draw
    void Init(const std::string& fileName, const size_t& num = 0) override;
    void Update() override;
    void Draw() override;

    // editor
    void AdjustParam() override;
    void BindParams() override;
    void AdjustUniqueParam() override;

 
private:
    std::unique_ptr<Sprite> symbolSprite_;
    Vector2 symbolPos_;
    Vector2 symbolScale_;
   
};