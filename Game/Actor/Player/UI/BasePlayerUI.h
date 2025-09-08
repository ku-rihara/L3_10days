#pragma once
#include "2d/Sprite.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <cstdint>
#include <memory>
#include <string>

class Player;
class BasePlayerUI {
public:
    BasePlayerUI()          = default;
    virtual ~BasePlayerUI() = default;

    // init,update,draw
    virtual void Init(const std::string& fileName, const size_t& num = 0);
    virtual void Update() = 0;
    virtual void Draw()   = 0;

    // editor
    virtual void AdjustParam();
    virtual void BindParams();
    virtual void AdjustUniqueParam() = 0;

protected:
    const std::string filePath_ = "Resources/Texture/PlayerUI/";

    GlobalParameter* globalParameter_;
    std::string groupName_;

    std::unique_ptr<Sprite> sprite_;
    Vector2 basePosition_;
    Vector2 baseScale_;

    Player* pPlayer_ = nullptr;

public:
    void SetPlayer(Player* player);
};