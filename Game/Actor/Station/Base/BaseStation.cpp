#include "Actor/Station/Base/BaseStation.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "Frame/Frame.h"
#include "Actor/NPC/NPC.h"
#include "3d/ViewProjection.h"
#include "random.h"
#include <algorithm>
#include "imgui.h"

namespace {
    inline bool NearerTo(const Vector3& p, NPC* a, NPC* b) {
        const float da = (a->GetWorldPosition() - p).Length();
        const float db = (b->GetWorldPosition() - p).Length();
        return da < db;
    }
}

BaseStation::BaseStation(const std::string& name)
    : name_(name) {}

BaseStation::~BaseStation() = default;

void BaseStation::Init() {
    // GlobalParameter
    globalParam_ = GlobalParameter::GetInstance();

    // メイングループ設定～同期
    globalParam_->CreateGroup(name_, true);
    globalParam_->ClearBindingsForGroup(name_);
    BindParms();
    globalParam_->SyncParamForGroup(name_);

    LoadData();

    // 初期座標
    baseTransform_.translation_ = initialPosition_;
    baseTransform_.UpdateMatrix();

    // === AI 初期化 ===
    ai_.Initialize(this, unitDirector_);
    ai_.SetConfig(aiCfg_);

    // 先に一度UpdateしてAI出力を初期化
    this->Update();

    // 初期スポーン
    StartupSpawn();

    // 再計画タイマ（起動直後に走らせる）
    planTimer_ = 0.0f;
}

void BaseStation::Update() {
    BaseObject::Update();

    const float dt = Frame::DeltaTime();
    currentTime_ += dt;

    // AI 更新
    ai_.SetRivalCached(pRivalStation_);
    ai_.UpdateWeighted(dt,
                       /*self*/  hp_, maxLife_,
                       /*pos*/   baseTransform_.translation_,
                       /*rival*/ pRivalStation_,
                       /*H*/     homeThreatDebug_);

                   // 役割再割当て
    planTimer_ -= dt;
    const float replan = ai_.Config().replanInterval; // GUI で編集可
    if (planTimer_ <= 0.0f) {
        ReassignRoles();
        planTimer_ = (replan > 0.05f) ? replan : 0.5f;
    }
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

    // ==== Yレンジ ====
    constexpr float kBoundaryY = 0.0f;
    constexpr float kMargin = 40.0f;

    float minY, maxY;
    if (c.y >= kBoundaryY) {
        minY = kBoundaryY + kMargin; // +側
        maxY = c.y;
    } else {
        minY = c.y;
        maxY = kBoundaryY - kMargin; // -側
    }
    if (maxY < minY) { minY = maxY = c.y; }

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
    // TODO: 弾衝突によるダメージ等
}

void BaseStation::ReassignRoles() {
    auto npcs = GetLiveNpcs();
    if (npcs.empty()) return;

    const auto out = ai_.GetLastOutput();
    const int N = static_cast<int>(npcs.size());

    int defenders = std::clamp(out.desiredDefenders, 0, N);
    int attackers = std::clamp(out.desiredAttackers, 0, N - defenders);

    const Vector3 home  = GetWorldPosition();
    const Vector3 rival = pRivalStation_ ? pRivalStation_->GetWorldPosition() : home;

    // 射撃候補の供給元
    for (auto* npc : npcs) if (npc) npc->SetTargetProvider(this);

    // 1) 防衛：自陣近い順
    std::sort(npcs.begin(), npcs.end(),
        [&](NPC* a, NPC* b){ return NearerTo(home, a, b); });

    for (int i = 0; i < defenders && i < (int)npcs.size(); ++i) {
        auto* npc = npcs[i];
        npc->SetTarget(nullptr);           // 攻撃目標解除
        npc->SetDefendAnchor(home);        // 自陣アンカーで防衛
    }

    // 2) 攻撃：残りを敵拠点に近い順
    std::vector<NPC*> rest(npcs.begin() + defenders, npcs.end());
    std::sort(rest.begin(), rest.end(),
        [&](NPC* a, NPC* b){ return NearerTo(rival, a, b); });

    for (int i = 0; i < attackers && i < (int)rest.size(); ++i) {
        auto* npc = rest[i];
        npc->ClearDefendAnchor();
        npc->SetTarget(pRivalStation_);
    }

    // 3) 余りは防衛に寄せる
    for (int i = attackers; i < (int)rest.size(); ++i) {
        auto* npc = rest[i];
        npc->SetTarget(nullptr);
        npc->SetDefendAnchor(home);
    }
}