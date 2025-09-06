#pragma once

// game
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "Actor/NPC/Navigation/MoveConstraint.h"
#include "Actor/Station/Base/BaseStation.h"
#include "BaseObject/BaseObject.h"

class FireController;

// 境界を壊す専用のnpc
class BoundaryBreaker : public BaseObject {
public:
	//===================================================================*/
	//		public functions
	//===================================================================*/
	BoundaryBreaker() = default;
	~BoundaryBreaker() override;

	void Init() override;
	void Update() override;

	// --------- accessor --------------------------------
	void SetFactionType(FactionType type);
	void SetAnchorPoint(const Vector3& point);
	void SetRivalStation(const BaseStation* station);
	// 均等配置のためのパラメータ
	void SetPhase(float rad);     //< 初期位相（ラジアン）
	void SetRadius(float r);      //< 回転半径

	float GetTurningRadius() const;

private:
	//===================================================================*/
	//		private functions
	//===================================================================*/
	void Move();		//< 移動
	void BindParms();	//< パラメータ
	void Shoot();		//< 弾発射

private:
	//===================================================================*/
	//		private variables
	//===================================================================*/
	bool isActive_ = true;
	std::string groupName_ = "BoundaryBreaker";

	// パラメータ
	float maxHp_ = 10.0f;			//< 最大ライフ
	float speed_ = 0.1f;			//< [rad/sec] を想定（※1秒で約45度 = 0.785 rad）
	float shootInterval_ = 5.0f;	//< 発射インターバル
	float shootCooldown_ = 1.0f;	//< 発射クールタイム
	float turningRadius_ = 45.0f;	//< 回転半径

	// 回転用（個体ごと）
	float angle_ = 0.0f;			//< 積算角
	float phase_ = 0.0f;			//< 初期位相

	// game
	Vector3 anchorPosition_{};
	FactionType faction_{};
	const BaseStation* pRivalStation_ = nullptr;
	GlobalParameter* globalParam_ = nullptr;
	std::unique_ptr<NpcFireController> fireController_;

	// 移動制御
	std::unique_ptr<IMoveConstraint> moveConstraint_ = nullptr;
};