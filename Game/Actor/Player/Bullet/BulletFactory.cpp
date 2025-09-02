#include "BulletFactory.h"
#include "PlayerMissile.h"
#include "PlayerNormalBullet.h"

std::unique_ptr<BasePlayerBullet> BulletFactory::CreateBullet(BulletType type) {
    switch (type) {
    case BulletType::NORMAL:
        return std::make_unique<PlayerNormalBullet>();

    case BulletType::MISSILE:
        return std::make_unique<PlayerMissile>();

    default:
        return nullptr;
    }
}