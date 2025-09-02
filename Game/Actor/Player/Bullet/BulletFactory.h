#pragma once
#include "BasePlayerBullet.h"
#include <memory>

class BulletFactory {
public:
    static std::unique_ptr<BasePlayerBullet> CreateBullet(BulletType type);
};