#pragma once
#include "BaseObject/BaseObject.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <cstdint>

class Player : public BaseObject {
public:
    Player()  = default;
    ~Player() = default;

    // 初期化、更新
    void Init();
    void Update();

    // Move
    void Move();

    // カメラから向きを決める
    void DirectionToCamera();

    // editor
    void AdjustParam();
    void BindParams();

private:

    // viewProjection
    const ViewProjection* viewProjection_ = nullptr;

    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Player";

    // Parameter
    int32_t hp_;
    float speed_;

    // direction
    Vector3 direction_;
    float objectiveAngle_;

public:
   /* void SetViewProjection(const ViewProjection);*/
};
