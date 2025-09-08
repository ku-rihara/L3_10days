#pragma once
#include "2d/Sprite.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <cstdint>
#include <memory>
#include <string>

class BasePlayerUI {
public:
    BasePlayerUI()          = default;
    virtual ~BasePlayerUI() = default;

    // init,update,draw
    virtual void Init(const std::string& fileName);
    virtual void Update() = 0;
    virtual void Draw()   = 0;

    // editor
    virtual void AdjustParam();
    virtual void BindParams();

private:
    const std::string filePath_ = "Resources/Texture/PlayerUI/";

protected:
    GlobalParameter* globalParameter_;
    std::string groupName_;

    std::unique_ptr<Sprite> sprite_;
    Vector2 basePosition_;
    Vector2 baseScale_;

public:
};