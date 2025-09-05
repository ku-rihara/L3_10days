#pragma once
#include "BaseObject/BaseObject.h"

class GlobalParameter;

class NpcBullet : public BaseObject {
public:
	/// ===================================================
	/// public func
	/// ===================================================
	NpcBullet() = default;
	virtual ~NpcBullet() override = default;

	virtual void Init() override;			// パラメータ束縛＋ロード
	void Init(const Vector3& dir);			// 方向指定で初期化
	void Update() override;

	// 方向（内部で正規化）＆寿命リセット
	void SetDirection(const Vector3& dir);

	// アクティブ制御（BaseObjectに無い前提で自前実装）
	void Activate()   noexcept { isActive_ = true;  lifeRemain_ = maxLifeTime_; }
	void Deactivate() noexcept { isActive_ = false; }
	bool IsAlive() const noexcept { return isActive_; }

protected:
	/// ===================================================
	/// protected Func
	/// ===================================================
	virtual void BindParms();
	void LoadData();
	void SaveData();

private:
	void Move();

protected:
	/// ===================================================
	/// protected Variables
	/// ===================================================
	// Param
	GlobalParameter* globalParam_ = nullptr;
	std::string groupName_ = "NpcBullet";
	std::string modelName_ = "cube.obj";	//< とりあえずデバッグ用
	const std::string fileDirectory_ = "GameActor/NpcBullet";

	float speed_ = 40.0f;		// m/s
	float maxLifeTime_ = 8.0f;	// s

	// Runtime
	Vector3 dir_{ 0,0,1 };
	float lifeRemain_ = 0.0f;
	float radius_ = 5.0f;		//< 弾の半径
	bool isActive_ = false;		//< アクティブフラグ
	bool isInitialized_ = false;

	Vector3 prevPos_{};
	float damage_ = 6.0f;
};