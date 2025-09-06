#include "Actor/Station/Base/BaseStation.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "Frame/Frame.h"
#include "Actor/NPC/NPC.h"
#include "3d/ViewProjection.h"
#include "random.h"
#include <algorithm>
#include "imgui.h"

BaseStation::BaseStation(const std::string& name)
	: name_(name) {}

BaseStation::~BaseStation() = default;

void BaseStation::Init() {
	// GlobalParameter 取得
	globalParam_ = GlobalParameter::GetInstance();

	// メイングループの設定～同期
	globalParam_->CreateGroup(name_, true);
	globalParam_->ClearBindingsForGroup(name_);
	BindParms();
	globalParam_->SyncParamForGroup(name_);

	LoadData();

	// 初期座標適用
	baseTransform_.translation_ = initialPosition_;
	baseTransform_.UpdateMatrix();

	// === AI 初期化 ===
	ai_.Initialize(this, unitDirector_);
	ai_.SetConfig(aiCfg_);

	this->Update();

	StartupSpawn();
}

void BaseStation::Update() {
	BaseObject::Update();
	// === AI 更新 ===
	const float dt = Frame::DeltaTime();
	ai_.SetRivalCached(pRivalStation_);
	ai_.UpdateWeighted(dt,
		/*self*/  hp_, maxLife_,
		/*pos*/   baseTransform_.translation_,
		/*rival*/ pRivalStation_,
		/*H*/     homeThreatDebug_);
}

void BaseStation::DrawDebug(const ViewProjection& vp) {
	for (auto& h : spawned_) h->DebugDraw(vp);
}

void BaseStation::ShowGui() {
	const std::string path = fileDirectory_;
	if (ImGui::CollapsingHeader(name_.c_str())) {
		ImGui::PushID(name_.c_str());

		globalParam_->ParamLoadForImGui(name_, path);
		ImGui::SameLine();
		globalParam_->ParamSaveForImGui(name_, path);

		// Transform
		if (ImGui::TreeNode("Transform")) {
			ImGui::DragFloat3("Scale", &baseTransform_.scale_.x, 0.01f);
			ImGui::DragFloat3("Rotate", &baseTransform_.rotation_.x, 0.01f);
			if (ImGui::DragFloat3("Translate", &baseTransform_.translation_.x, 0.01f)) {
				initialPosition_ = baseTransform_.GetWorldPos();
			}
			ImGui::TreePop();
		}

		// Station AI
		if (ImGui::CollapsingHeader("Station AI")) {
			auto& c = ai_.Config();
			ImGui::DragFloat("minAttack", &c.minAttack, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("maxAttack", &c.maxAttack, 0.01f, 0.0f, 1.0f);
			ImGui::DragInt("minDefenders", &c.minDefenders, 1, 0, 999);
			ImGui::DragFloat("temperature", &c.temperature, 0.01f, 0.05f, 3.0f);
			ImGui::DragFloat("refAttackRange", &c.refAttackRange, 1.0f, 10.0f, 5000.0f);
			ImGui::DragFloat("dpsNormSelf", &c.dpsNormSelf, 0.01f, 0.05f, 1.0f);
			ImGui::DragFloat("dpsNormRival", &c.dpsNormRival, 0.01f, 0.05f, 1.0f);
			ImGui::DragFloat("smoothingSelf", &c.smoothingSelf, 0.1f, 0.5f, 10.0f);
			ImGui::DragFloat("smoothingRival", &c.smoothingRival, 0.1f, 0.5f, 10.0f);
			ImGui::DragFloat("replanInterval", &c.replanInterval, 0.05f, 0.1f, 3.0f);
			ImGui::DragFloat("ratioDeadband", &c.ratioDeadband, 0.005f, 0.0f, 0.5f);
			ImGui::SeparatorText("Weights");
			ImGui::DragFloat("w.selfLowHp", &c.w.selfLowHp, 0.05f, 0.0f, 3.0f);
			ImGui::DragFloat("w.rivalLowHp", &c.w.rivalLowHp, 0.05f, 0.0f, 3.0f);
			ImGui::DragFloat("w.selfDmgRate", &c.w.selfDmgRate, 0.05f, 0.0f, 3.0f);
			ImGui::DragFloat("w.rivalDmgRate", &c.w.rivalDmgRate, 0.05f, 0.0f, 3.0f);
			ImGui::DragFloat("w.homeThreat", &c.w.homeThreat, 0.05f, 0.0f, 3.0f);
			ImGui::DragFloat("w.distance", &c.w.distance, 0.05f, 0.0f, 3.0f);
			ImGui::DragFloat("w.biasAttack", &c.w.biasAttack, 0.05f, -2.0f, 2.0f);

			ImGui::Separator();
			ImGui::SliderFloat("homeThreat (debug)", &homeThreatDebug_, 0.0f, 1.0f);

			const auto& out = ai_.GetLastOutput();
			ImGui::Text("Attack Ratio     : %.2f", out.attackRatio);
			ImGui::Text("Desired Defenders: %d", out.desiredDefenders);
			ImGui::Text("Desired Attackers: %d", out.desiredAttackers);
		}

		ImGui::PopID();
	}
}

void BaseStation::StartupSpawn() {
	const int target = std::clamp(initialSpawnCount_, 0, maxConcurrentUnits_);
	const float R = initialSpawnDistanceFromThis_;
	const Vector3 c = GetWorldPosition();

	// ==== Y のレンジを決定 ====
	constexpr float kBoundaryY = 0.0f;
	constexpr float kMargin = 40.0f;

	float minY, maxY;
	if (c.y >= kBoundaryY) {
		// +Y側陣営
		minY = kBoundaryY + kMargin; // 40 以上上
		maxY = c.y;                  // 自陣の高さ
	} else {
		// -Y側陣営
		minY = c.y;                  // 自陣の高さ
		maxY = kBoundaryY - kMargin; // -40 以下
	}

	// もし範囲が成立しない（自陣が境界に近すぎるなど）場合は拠点Y固定
	if (maxY < minY) {
		minY = maxY = c.y;
	}

	for (int i = 0; i < target; ++i) {
		float x = c.x + Random::Range(-R, R);
		float z = c.z + Random::Range(-R, R);
		float y = Random::Range(minY, maxY);

		SpawnNPC(Vector3{ x, y, z });
	}
}

void BaseStation::BindParms() {
	globalParam_->Bind(name_, "initialPosition", &initialPosition_);
}

void BaseStation::LoadData() {
	const std::string path = fileDirectory_;
	globalParam_->LoadFile(name_, path);
	globalParam_->SyncParamForGroup(name_);
}

void BaseStation::SaveData() {
	const std::string path = fileDirectory_;
	globalParam_->SaveFile(name_, path);
}

// accessor
void BaseStation::SetRivalStation(BaseStation* rival) { pRivalStation_ = rival; }
BaseStation* BaseStation::GetRivalStation() const { return pRivalStation_; }
void BaseStation::SetFaction(FactionType type) { faction_ = type; }
FactionType BaseStation::GetFactionType() const { return faction_; }

// NPC 管理
std::vector<NPC*> BaseStation::GetLiveNpcs() const {
	std::vector<NPC*> out; out.reserve(spawned_.size());
	for (auto& h : spawned_) if (h) out.push_back(h.get());
	return out;
}

void BaseStation::CollectTargets(std::vector<const BaseObject*>& out) const {
	out.clear();

	if (pRivalStation_) {
		for (NPC* npc : pRivalStation_->GetLiveNpcs()) {
			if (!npc) continue;
			out.push_back(static_cast<const BaseObject*>(npc));
		}

		out.push_back(static_cast<const BaseObject*>(pRivalStation_));
	}
}

void BaseStation::OnCollisionEnter(BaseCollider* /*other*/) {
	/// TODO: 衝突した相手が弾ならダメージを受ける

}
