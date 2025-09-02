#include "NPC.h"

#include "Actor/Station/Base/BaseStation.h"
#include "Actor/Boundary/Boundary.h"
#include "Frame/Frame.h"

#include <algorithm>
#include <cmath>
#include <limits>

// ===== BoundaryHoleSource 実装 =====
const std::vector<Hole>& NPC::BoundaryHoleSource::GetHoles() const {
	static const std::vector<Hole> kEmpty;
	return boundary ? boundary->GetHoles() : kEmpty;
}

/// ===================================================
/// 初期化
/// ===================================================
void NPC::Init() {
	globalParam_ = GlobalParameter::GetInstance();

	// グループ名が入っていなければデフォルト
	if (groupName_.empty()) { groupName_ = "UnnamedNPC"; }

	// メイングループの設定
	globalParam_->CreateGroup(groupName_, true);

	// 重複バインドを防ぐ
	globalParam_->ClearBindingsForGroup(groupName_);
	BindParms();

	// パラメータ同期
	globalParam_->SyncParamForGroup(groupName_);

	// LoadData(); // 必要なら

	Activate();

	// 航法初期化（自分の位置を中心に旋回開始）
	navigator_.Reset(GetWorldPosition());

	// --- 通行制約（XZ 長方形＋穴ゲート）をセットアップ ---
	// あなたの Boundary メッシュは x,z ∈ [-1500,1500] の板なので、それを Rect に反映
	Boundary* boundary = Boundary::GetInstance();
	holeSource_.boundary = boundary;

	RectXZ rect{ -1500.0f, 1500.0f, -1500.0f, 1500.0f };
	moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_, rect, 0.01f);

	isInitialized_ = true;
}

/// ===================================================
/// 更新
/// ===================================================
void NPC::Update() {
	Move();
	BaseObject::Update();
}

/// ===================================================
/// 移動（Navigator 出力 → Constraint で検閲 → 反映）
/// ===================================================
void NPC::Move() {
	if (!isActive_) return;

	const float dt = Frame::DeltaTime();

	const Vector3 npcPos = GetWorldPosition();
	const Vector3 tgtPos = (target_) ? target_->GetWorldPosition() : npcPos;

	// 航法ロジック：望ましい「移動量（速度×dt）」を得る
	const Boundary* boundary = Boundary::GetInstance();
	const std::vector<Hole>& holes = boundary->GetHoles();

	if (speed_ != navConfig_.speed) {
		navConfig_.speed = speed_;
	}

	const Vector3 desiredDelta = navigator_.Tick(dt, npcPos, tgtPos, holes);

	Vector3 from = baseTransform_.translation_;
	Vector3 to = from + desiredDelta;

	if (moveConstraint_) {
		to = moveConstraint_->FilterMove(from, to);
	}

	baseTransform_.translation_ = to;

	if (!target_) {
		StartOrbit(npcPos);
	}
}

/// ===================================================
/// 旋回開始（Navigator へ委譲）
/// ===================================================
void NPC::StartOrbit(const Vector3& center) {
	navigator_.StartOrbit(center);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      パラメータ
/////////////////////////////////////////////////////////////////////////////////////////

void NPC::BindParms() {
	globalParam_->Bind(groupName_, "maxHP", &maxHP_);
	globalParam_->Bind(groupName_, "speed", &speed_);

	// 必要に応じて Navigator 設定も GUI で触れるように
	// globalParam_->Bind(groupName_, "arriveSlowRadius",  &navConfig_.arriveSlowRadius);
	// globalParam_->Bind(groupName_, "minHoleRadius",     &navConfig_.minHoleRadius);
	// globalParam_->Bind(groupName_, "passFrac",          &navConfig_.passFrac);
	// globalParam_->Bind(groupName_, "retargetInterval",  &navConfig_.retargetInterval);
	// globalParam_->Bind(groupName_, "orbitRadius",       &navConfig_.orbitRadius);
	// globalParam_->Bind(groupName_, "orbitAngularSpd",   &navConfig_.orbitAngularSpd);
}

void NPC::LoadData() { globalParam_->LoadFile(groupName_, fileDirectory_); }
void NPC::SaveData() { globalParam_->SaveFile(groupName_, fileDirectory_); }

/////////////////////////////////////////////////////////////////////////////////////////
//      accessor
/////////////////////////////////////////////////////////////////////////////////////////

void NPC::SetTarget(const BaseStation* target) { target_ = target; }
void NPC::SetFaction(FactionType faction) { faction_ = faction; }
void NPC::Activate() { isActive_ = true; }
void NPC::Deactivate() { isActive_ = false; }
