#pragma once
#include "3d/Object3d.h"
#include "3d/WorldTransform.h"
#include"Easing/Easing.h"
#include"Easing/EasingSequence.h"
class EasingTestObj {
private:

    enum class AdaptType {
        Position, // 位置
        Rotation, // 回転
        Scale, // 拡大縮小
	};

private:
	
	// モデル
	std::unique_ptr<Object3d> object3D_=nullptr;

	Easing<Vector3> easing_;

	
	
public:
    EasingTestObj();
    ~EasingTestObj();
	void Init();
	void Update();
	
	void Debug();
	void ScaleEasing();

	 Easing<Vector3>& GetEasingData()  { return easing_; }
};
