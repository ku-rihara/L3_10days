#pragma once

enum class BulletType {
    NORMAL,
    MISSILE,
    COUNT,
};

struct BulletParameter {
    float lifeTime;
    float speed;
    bool isHoming;
};

class BasePlayerBullet {
public:
    

public:
    virtual ~BasePlayerBullet() = default;

protected:
    BulletParameter param_;
    BulletType type_;

public:
    void SetParameter(const BulletType& type, const BulletParameter& parameter);
};