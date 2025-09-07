#pragma once
#include "3d/WorldTransform.h"
#include "BaseObject/BaseObject.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Vector3.h"
#include <string>

class BasePlayerParts : public BaseObject {
public:
    BasePlayerParts()          = default;
    virtual ~BasePlayerParts() =  default;
    

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

public:
    void SetObjQuaternion(const Quaternion& q) { obj3d_->transform_.quaternion_ = q; }
};