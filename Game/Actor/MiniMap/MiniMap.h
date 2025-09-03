#pragma once

/// std
#include <memory>

/// game
#include "3d/BaseObject3d.h"
#include "2d/Sprite.h"
#include "Pipeline/Buffer/ConstantBuffer.h"
#include "Pipeline/Buffer/StructuredBuffer.h"

struct MiniMapBufferData {
	Matrix4x4 matrix; /// SRT
};

struct IconBufferData {
	Matrix4x4 matrix; /// SRT
};

/// //////////////////////////////////////////////////////
/// プレイヤーを中心としたミニマップ
/// //////////////////////////////////////////////////////
class MiniMap {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	MiniMap() = default;
	~MiniMap() = default;

	void Init();
	void Update();

	void DrawMiniMap();

	/// ----- Playerや味方、敵の登録/解除 ----- ///
	void RegisterPlayer(BaseObject3d* _player);
	void AddFriend(BaseObject3d* _friend);
	void RemoveFriend(BaseObject3d* _friend);
	void AddEnemy(BaseObject3d* _enemy);
	void RemoveEnemy(BaseObject3d* _enemy);


	/// ----- Bufferの取得
	ConstantBuffer<MiniMapBufferData>& GetMiniMapBuffer();
	StructuredBuffer<IconBufferData>& GetFriendIconBuffer();
	StructuredBuffer<IconBufferData>& GetEnemyIconBuffer();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::unique_ptr<Sprite> miniMapFrameSprite_;

	/// 表示対象のオブジェクト
	BaseObject3d* playerPtr_;
	std::vector<BaseObject3d*> friends_; /// 味方
	std::vector<BaseObject3d*> enemies_; /// 敵

	/// Buffer
	ConstantBuffer<MiniMapBufferData> miniMapBuffer_;
	StructuredBuffer<IconBufferData> friendIconBuffer_;
	StructuredBuffer<IconBufferData> enemyIconBuffer_;
};

