#include "NPC.h"

#include "Actor/Station/Base/BaseStation.h"
#include "Actor/Boundary/Boundary.h"
#include "Frame/Frame.h"

/// ===================================================
/// 初期化
/// ===================================================
void NPC::Init() {

	globalParam_ = GlobalParameter::GetInstance();

	// グループ名が入っていなければデフォルト
	if (groupName_.empty()) { groupName_ = "UnnamedNPC"; }

	//メイングループの設定
	globalParam_->CreateGroup(groupName_, true);

	//重複バインドを防ぐ
	globalParam_->ClearBindingsForGroup(groupName_);
	BindParms();

	//パラメータ同期
	globalParam_->SyncParamForGroup(groupName_);

	//LoadData();

	Activate();
}

/// ===================================================
/// 移動
/// ===================================================
void NPC::Move() {
	const float dt = Frame::DeltaTime();

	// リターゲットクールダウン
	if (retargetCooldown_ > 0.0f) retargetCooldown_ -= dt;

	// ターゲットが無ければ旋回
	if (!target_) {
		state_ = NavState::Orbit;
	}

	const Vector3 npcPos = GetWorldPosition();
	const Vector3 tgtPos = target_ ? target_->GetWorldPosition() : npcPos;

	Boundary* boundary = Boundary::GetInstance();
	const std::vector<Hole>& holes = boundary->GetHoles();

	// 有効な穴の有無
	auto hasValidHole = [&]() {
		for (const auto& h : holes) if (h.radius >= minHoleRadius_) return true;
		return false;
	};

	// 現在の穴が無効なら破棄
	auto invalidateCurrentHole = [&]() {
		currentHoleIndex_ = -1;
		currentHolePos_ = {};
		currentHoleRadius_ = 0.0f;
	};

	// 穴選択（必要時のみ）
	auto tryRetargetHole = [&]() {
		if (retargetCooldown_ > 0.0f) return;

		int best = SelectBestHole(holes, npcPos, tgtPos);
		if (best >= 0) {
			currentHoleIndex_ = best;
			currentHolePos_ = holes[best].position;
			currentHoleRadius_ = holes[best].radius;
			state_ = NavState::ToHole;
			retargetCooldown_ = retargetInterval_;
		} else {
			// 選べなかったら旋回
			StartOrbit(npcPos);
		}
	};

	// 状態遷移＆移動
	switch (state_) {
		case NavState::Orbit:
			{
				// 穴が出現したら狙う
				if (hasValidHole()) {
					tryRetargetHole();
				}

				// 旋回運動（実際に移動する）
				orbitAngle_ += orbitAngularSpeed_ * dt;
				Vector3 desired = orbitCenter_ + Vector3(std::cos(orbitAngle_), 0.0f, std::sin(orbitAngle_)) * orbitRadius_;
				Vector3 dir = desired - npcPos;
				float len = dir.Length();
				if (len > 1e-4f) dir = dir * (1.0f / len);
				baseTransform_.translation_ += dir * speed_ * dt;
			}
			break;

		case NavState::ToHole:
			{
				// 現行の穴がまだ有効かチェック（holesベクタは消滅で詰められるのでindex検証）
				bool holeOk = false;
				if (currentHoleIndex_ >= 0 && currentHoleIndex_ < static_cast<int>(holes.size())) {
					const Hole& h = holes[currentHoleIndex_];
					holeOk = (h.radius >= minHoleRadius_);
					currentHolePos_ = h.position; // 念のため最新化
					currentHoleRadius_ = h.radius;
				}

				if (!holeOk) {
					// 近い別の穴へ切替を試みる。なければ旋回。
					invalidateCurrentHole();
					tryRetargetHole();
					if (state_ == NavState::Orbit) break; // 旋回へ
				}

				// 穴へ向かう
				Vector3 toHole = currentHolePos_ - npcPos;
				float   distH = toHole.Length();
				if (distH > 1e-4f) toHole = toHole * (1.0f / distH);

				// 到達判定（穴の半径×passFrac_）
				float reachDist = (std::max)(1.0f, currentHoleRadius_ * passFrac_);
				if (distH <= reachDist) {
					// 「穴を通過した」→ターゲットへ
					state_ = NavState::ToTarget;
					invalidateCurrentHole();
				} else {
					// ふつうに移動
					baseTransform_.translation_ += toHole * speed_ * dt;
				}
			}
			break;

		case NavState::ToTarget:
			{
				// 新たに良い穴が出たらそちらを狙い直す（任意）
				if (hasValidHole()) {
					tryRetargetHole();         // 条件が揃えばToHoleへ遷移
					if (state_ == NavState::ToHole) break;
				}

				Vector3 toTgt = tgtPos - npcPos;
				float   dist = toTgt.Length();
				if (dist > 1e-4f) toTgt = toTgt * (1.0f / dist); // /= は使わない

				// 簡易アライブ（近くで減速したいなら）
				float speedScale = 1.0f;
				if (dist < arriveSlowRadius_) speedScale = dist / (std::max)(1e-3f, arriveSlowRadius_);

				baseTransform_.translation_ += toTgt * speed_ * speedScale * dt;

				// 「穴無し時はターゲットへ向かわない」なら Orbit に戻す
				if (!hasValidHole()) {
					StartOrbit(npcPos);
				}
			}
			break;
	}
}


int NPC::SelectBestHole(const std::vector<Hole>& holes,
						const Vector3& npcPos,
						const Vector3& tgtPos) const {
	if (holes.empty()) return -1;

	const Vector3 toTgt = tgtPos - npcPos;
	float toTgtLen = toTgt.Length();
	Vector3 toTgtDir = (toTgtLen > 1e-4f) ? (toTgt * (1.0f / toTgtLen)) : Vector3{ 0,0,1 };

	int   bestIdx = -1;
	float bestCost = (std::numeric_limits<float>::max)();

	for (int i = 0; i < static_cast<int>(holes.size()); ++i) {
		const Hole& h = holes[i];
		if (h.radius < minHoleRadius_) continue;

		Vector3 toHole = h.position - npcPos;
		float   thLen = toHole.Length();
		if (thLen < 1e-4f) continue;
		Vector3 toHoleDir = toHole * (1.0f / thLen);   // / ではなく *

		float cosA = Vector3::Dot(toHoleDir, toTgtDir);
		if (cosA < 0.0f) continue; // 反対側は除外

		// コスト：NPC→穴 + 穴→ターゲット（わずかに重み）
		float cost = thLen + (tgtPos - h.position).Length() * 1.05f;

		if (cost < bestCost) {
			bestCost = cost;
			bestIdx = i;
		}
	}
	return bestIdx;
}

// =========================
// 旋回開始
// =========================
void NPC::StartOrbit(const Vector3& center) {
	state_ = NavState::Orbit;
	orbitCenter_ = center;
	orbitAngle_ = 0.0f;
	retargetCooldown_ = retargetInterval_;
}

/// ===================================================
/// 更新
/// ===================================================
void NPC::Update() {
	Move();
	BaseObject::Update();
}

/////////////////////////////////////////////////////////////////////////////////////////
//		パラメータ
/////////////////////////////////////////////////////////////////////////////////////////

/// ===================================================
/// パラメータの同期
/// ===================================================
void NPC::BindParms() { globalParam_->Bind(groupName_, "maxHP", &maxHP_); }

/// ===================================================
/// データ読み込み
/// ===================================================
void NPC::LoadData() {
	globalParam_->LoadFile(groupName_, fileDirectory_);
}

/// ===================================================
/// データ保存
/// ===================================================
void NPC::SaveData() { globalParam_->SaveFile(groupName_, fileDirectory_); }


/////////////////////////////////////////////////////////////////////////////////////////
//		accessor
/////////////////////////////////////////////////////////////////////////////////////////

/// ===================================================
///  ターゲットを設定
/// ===================================================
void NPC::SetTarget(const BaseStation* target) { target_ = target; }

/// ===================================================
///  派閥を設定
/// ===================================================
void NPC::SetFaction(FactionType faction) { faction_ = faction; }

/// ===================================================
///  アクティブトグル
/// ===================================================
void NPC::Activate() { isActive_ = true; }

void NPC::Deactivate() { isActive_ = false; }


