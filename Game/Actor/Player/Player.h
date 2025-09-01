#pragma once
#include "utility/ParameterEditor/GlobalParameter.h"
#include "BaseObject/BaseObject.h"
#include <cstdint>

class Player : public BaseObject {
public:
    Player()  = default;
    ~Player() = default;

    // 初期化、更新
    void Init();
    void Update();

    void Move();

    // カメラから向きを決める
    void DirectionToCamera();

    // editor
    void AdjustParam();
    void BindParams();

private:
    const ViewProjection* viewProjection_ = nullptr;

    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Player"; 

    // direction
    Vector3 direction_;
    float objectiveAngle_;

    // Parameter
    int32_t hp_;
    float speed_;
    

};
