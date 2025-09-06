#pragma once

// engine
#include "utility/ParameterEditor/GlobalParameter.h"

// game
#include "BaseObject/BaseObject.h"
#include "Details/Faction.h"
#include "Actor/NPC/Pool/NpcPool.h"

// AI
#include "Actor/Station/UnitDirector/IUnitDirector.h"
#include "AI/Station/StationAI.h"

// targeting
#include "../../NPC/Bullet/Targeting.h"

// std
#include <string>
#include <vector>

class NPC;

class BaseStation : public BaseObject, public ITargetProvider {
public:
	BaseStation(const std::string& name = "UnnamedStation");
	virtual ~BaseStation() override;

	virtual void Init() override;
	virtual void Update() override;
	void DrawDebug(const class ViewProjection&);
	virtual void ShowGui();

	virtual void StartupSpawn();

	// ---- 調整項目 ----
	virtual void BindParms();
	virtual void LoadData();
	virtual void SaveData();

	// ---- accessor ----
	void SetRivalStation(BaseStation* rival);
	BaseStation* GetRivalStation() const;
	void SetFaction(FactionType type);
	FactionType GetFactionType() const;

	// AI用アクセサ
	void    SetUnitDirector(IUnitDirector* d) { unitDirector_ = d; }
	float   GetHp()       const { return hp_; }
	float   GetMaxHp()    const { return maxLife_; }
	Vector3 GetPosition() const { return baseTransform_.translation_; } // 必要なら GetWorldPos() に変更

	// NPC 管理
	std::vector<NPC*> GetLiveNpcs() const;

	template<class Fn>
	void ForEachNpc(Fn&& fn) {
		for (auto& h : spawned_) if (h) fn(*h.get());
	}

	// ===== ITargetProvider 実装 =====
	// この Station にとっての「敵候補」を out に詰める（Rival 側の NPC / Station 本体など）
	void CollectTargets(std::vector<const BaseObject*>& out) const override;

protected:
	virtual void SpawnNPC(const Vector3& pos) = 0;   //< npcをスポーン
	void CleanupSpawnedList();                       //< リストの掃除

protected:
	// 調整用
	GlobalParameter* globalParam_{};			//< 調整項目用（Initで取得）
	const std::string  name_;					//< 調整項目グループ名
	const std::string  fileDirectory_ = "GameActor/Station";

	// パラメータ
	Vector3 initialPosition_{};
	float   maxLife_ = 100.0f;
	float   spawnInterbal_ = 5.0f;
	int     maxConcurrentUnits_ = 20;
	int		initialSpawnCount_ = 20;
	float initialSpawnDistanceFromThis_ = 1500.0f;	//< 初期配置用

	// ---- game ----
	FactionType faction_{};
	float   hp_ = 100.0f;
	float   currentTime_ = 0.0f;

	std::vector<NpcHandle> spawned_;
	BaseStation* pRivalStation_ = nullptr; //< ライバル拠点

	// === AI関連 ===
	IUnitDirector* unitDirector_ = nullptr;
	StationAI       ai_;
	StationAiConfig aiCfg_;
	float           homeThreatDebug_ = 0.0f; // 0..1 自陣圧デバッグ入力
};