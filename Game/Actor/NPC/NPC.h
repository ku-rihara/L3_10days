#pragma once
// engine
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Collider/AABBCollider.h"

// game
#include "BaseObject/BaseObject.h"
#include "Details/Faction.h"
#include "Navigation/NpcNavigator.h"
#include "Navigation/MoveConstraint.h"
// targeting
#include "Bullet/Targeting.h"

#include <vector>
#include <string>
#include <memory>

class BaseStation;
class Line3D;
class Boundary;
class NpcFireController;
struct Hole;

class NPC : public BaseObject, public AABBCollider {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	NPC();
	~NPC() override;

	void Init() override;
	void Update() override;
	void DebugDraw(const class ViewProjection& vp);

	// --------- accessor --------------------------------
	void SetTarget(const BaseStation* target);  // 攻撃先（敵拠点）。nullptrで解除＝防衛側へ
	void SetFaction(FactionType faction) { faction_ = faction; }
	FactionType GetFaction() const { return faction_; }
	bool GetIsAlive() const { return isActive_; }

	void Activate();
	void Deactivate();

	// 防衛アンカー（防衛時の旋回中心）を指定/解除
	void SetDefendAnchor(const Vector3& p);
	void ClearDefendAnchor();

	// 射撃モード（直進/ホーミング）
	enum class FireMode { Straight, Homing };
	void SetFireMode(FireMode m) noexcept { fireMode_ = m; }
	FireMode GetFireMode() const noexcept { return fireMode_; }

	// ターゲット供給元の差し込み（Station/Director 等で実装して渡す）
	void SetTargetProvider(const ITargetProvider* p) noexcept { targetProvider_ = p; }

	// ---- 調整項目 ----
	virtual void BindParms();
	virtual void LoadData();
	virtual void SaveData();

	/// ===================================================
	/// collision methods
	/// ===================================================
	void OnCollisionEnter(BaseCollider* other) override;

public:
	void SetRole(NpcNavigator::Role r){ navigator_.SetRole(r); }

private:
	/// ===================================================
	///  private func（末尾アンダースコア無し）
	/// ===================================================
	void TryFire();                          // 発砲判定と弾生成
	void Move();                             // 航法（Navigator を使って移動）
	void StartOrbit(const Vector3& center);  // ロイター開始

	// 視錐台チェック & 視錐台から最適ターゲットを選ぶ
	bool IsInFiringFrustum(const Vector3& worldPt) const;
	const BaseObject* PickFrustumTarget() const;

protected:
	/// ===================================================
	///  protected-like variable
	/// ===================================================
	std::unique_ptr<NpcFireController> fireController_ = nullptr;

	// ---- param ----
	GlobalParameter* globalParam_ = nullptr;   //< 調整項目用
	std::string groupName_ = "NPC";            //< 調整項目グループ名
	const std::string fileDirectory_ = "GameActor/NPC";

	// パラメータ
	float maxHP_ = 10.0f;				//< 最大hp
	float hp_;							//< 現在のhp
	float speed_ = 18.0f;				//< 基本移動速度（Navigator に渡す）
	float shootInterval_ = 1.2f;		//< 発射間隔
	float shootCooldown_ = 0.0f;		//< 残りクールダウン

	// ---- firing cone（前方視錐台）----
	float fireConeNear_ = 2.0f;			//< 最短距離
	float fireConeFar_ = 120.0f;		//< 射程
	float fireConeHFovDeg_ = 25.0f;		//< 水平ハーフFOV（度）
	float fireConeVFovDeg_ = 15.0f;		//< 垂直ハーフFOV（度）

	// ---- game ----
	const BaseStation* target_ = nullptr;            // 航法上の目標（敵拠点）。nullptr で防衛モード
	const ITargetProvider* targetProvider_ = nullptr;
	FireMode fireMode_ = FireMode::Straight;          // 射撃モード

	FactionType faction_{};
	bool isActive_ = true;
	bool isInitialized_ = false;

	// 防衛アンカー
	bool    hasDefendAnchor_ = false;
	Vector3 defendAnchor_{};

	// ---- 航法（純ロジック）----
	NpcNavConfig navConfig_{
		/* speed            */ 18.0f,
		/* minSpeed         */ 5.0f,
		/* maxSpeed         */ 200.0f,
		/* orbitClockwise   */ +1,     // +1: 時計回り（必要に応じて -1 に）
		/* orbitRadius      */ 30.0f,
		/* orbitRadialGain  */ 0.5f,
		/* orbitTangentBias */ 1.0f,
		/* orbitAngularSpd  */ 0.0f,
		/* maxTurnRateDeg   */ 120.0f,
		/* passFrac         */ 0.25f,
		/* minHoleRadius    */ 3.0f
	};
	NpcNavigator navigator_{ navConfig_ };

	// ---- 制約 ----
	struct BoundaryHoleSource : IHoleSource {
		const Boundary* boundary = nullptr;
		const std::vector<Hole>& GetHoles() const override;
	} holeSource_;

	std::unique_ptr<IMoveConstraint> moveConstraint_;// 移動制御（任意）

	std::unique_ptr<Line3D> lineDrawer_;
};
