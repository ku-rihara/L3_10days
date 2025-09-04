#pragma once
#include "BaseObject/BaseObject.h"

class GlobalParameter;

class NpcBullet : public BaseObject {
public:
	/// ===================================================
	/// public func
	/// ===================================================
	NpcBullet() = default;
	~NpcBullet() override = default;

	void Init() override;                 // パラメータ束縛＋ロード
	void Init(const Vector3& dir);        // 方向指定で初期化
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
	void BindParms();
	void LoadData();
	void SaveData();

private:
	void Move();

private:
	/// ===================================================
	/// private Variables
	/// ===================================================
	// Param
	GlobalParameter* globalParam_ = nullptr;
	std::string groupName_ = "NpcBullet";
	const std::string fileDirectory_ = "GameActor/NpcBullet";

	float speed_ = 40.0f;  // m/s
	float maxLifeTime_ = 2.0f;   // s

	// Runtime
	Vector3 dir_{ 0,0,1 };
	float   lifeRemain_ = 0.0f;
	bool    isActive_ = false;  // ← 自前のアクティブフラグ
	bool    isInitialized_ = false;
};
