#pragma once
#include "BaseObject/BaseObject.h"

class TestGround : public BaseObject {
public:
    TestGround() = default;
    ~TestGround() = default;

    // 初期化、更新
    void Init();
    void Update();

private:
};
