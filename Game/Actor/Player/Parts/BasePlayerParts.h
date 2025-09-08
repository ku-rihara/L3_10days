#pragma once
#include "3d/WorldTransform.h"
#include "BaseObject/BaseObject.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Vector3.h"
#include <string>

class BasePlayerParts : public BaseObject {
public:
    BasePlayerParts()          = default;
    virtual ~BasePlayerParts() = default;

    virtual void Init(WorldTransform* transform, const std::string& GroupName);
    virtual void Update() = 0;

    // editor
    virtual void AdjustParam();
    void BindParams();

protected:
    // globalParameter
    GlobalParameter* globalParameter_;
    std::string groupName_;

    Vector3 offsetPos_;

    Vector3 baseRotate_;

    // 入力による回転制御
    Vector3 inputRotation_;
    Vector3 targetInputRotation_;
    float inputRotationSpeed_; // 回転速度
    float returnSpeed_; // 元に戻る速度

public:
    void SetBaseRotate(const Vector3& q) { baseRotate_ = q; }
    void SetInputRotation(const Vector3& input) { targetInputRotation_ = input; }
};