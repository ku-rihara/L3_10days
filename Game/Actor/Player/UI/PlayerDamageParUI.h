#pragma once
#include "2d/Sprite.h"
#include "BasePlayerUI.h"
#include <array>
#include <memory>
#include <string>

class PlayerDamageParUI : public BasePlayerUI {
public:
    PlayerDamageParUI()           = default;
    ~PlayerDamageParUI() override = default;

    // init,update,draw
    void Init(const std::string& fileName, const size_t& num = 0) override;
    void Update() override;
    void Draw() override;

    // editor
    void AdjustParam() override;
    void BindParams() override;
    void AdjustUniqueParam() override;

private:
    void UpdateDigitSprites();
    void SetDigitUV(std::unique_ptr<Sprite>& sprite, int digit);
    int CalculateDamagePercentage() const;

private:
    static constexpr int MAX_DIGITS = 3; // 0-100%なので最大3桁
    std::array<std::unique_ptr<Sprite>, MAX_DIGITS> digitSprites_;
    std::unique_ptr<Sprite> percentSprite_; // %マーク用

    Vector2 digitSpacing_  = Vector2(32.0f, 0.0f);
    Vector2 percentOffset_ = Vector2(64.0f, 0.0f);
    Vector2 digitScale_    = Vector2(1.0f, 1.0f); 
    Vector2 percentScale_  = Vector2(1.0f, 1.0f); 
};