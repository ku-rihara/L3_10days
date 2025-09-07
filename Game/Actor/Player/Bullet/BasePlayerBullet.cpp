#include "BasePlayerBullet.h"



BasePlayerBullet::BasePlayerBullet() {
    AABBCollider::Init();
}

void BasePlayerBullet::Init() {}

void BasePlayerBullet::SetParameter(const BulletType& type, const BulletParameter& parameter) {
    type_      = type;
    param_ = parameter;
}

