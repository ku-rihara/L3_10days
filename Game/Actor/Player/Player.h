#pragma once
#include"BaseObject/BaseObject.h"

class Player : public BaseObject {
public:

    Player()  = default;
    ~Player() = default;

    //初期化、更新
    void Init();
    void Update();

private:

};
