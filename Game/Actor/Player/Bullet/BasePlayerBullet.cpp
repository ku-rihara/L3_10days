#include "BasePlayerBullet.h"



void BasePlayerBullet::SetParameter(const BulletType& type, const BulletParameter& parameter) {
    type_      = type;
    param_ = parameter;
}