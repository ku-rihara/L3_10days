#pragma once

/// std
#include <memory>

/// game
#include "BaseObject/BaseObject.h"
#include "2d/Sprite.h"
#include "Pipeline/Buffer/ConstantBuffer.h"
#include "Pipeline/Buffer/StructuredBuffer.h"

struct MiniMapSize {
	Vector2 min;
	Vector2 max;
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

	void Init(class BaseStation* _ally, class BaseStation* _enemy);
	void Update();

	void DrawMiniMap();

	/// ----- Playerや味方、敵の登録/解除 ----- ///
	void RegisterPlayer(BaseObject* _player);

	/// ----- Bufferの取得
	ConstantBuffer<MiniMapSize>& GetMiniMapSizeBuffer();
	StructuredBuffer<IconBufferData>& GetFriendIconBuffer();
	StructuredBuffer<IconBufferData>& GetEnemyIconBuffer();

	UINT GetFriendIconCount() const;
	UINT GetEnemyIconCount() const;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	/// ----- other ----- ///
	class BaseStation* pAllyStation_;
	class BaseStation* pEnemyStation_;
	std::unique_ptr<Sprite> miniMapFrameSprite_;
	std::unique_ptr<Sprite> miniMapPlayerIconSprite_;

	Vector2 miniMapPos_;  /// ミニマップの位置

	/// 表示対象のオブジェクト
	BaseObject* playerPtr_;
	std::vector<BaseObject*> friends_; /// 味方
	std::vector<BaseObject*> enemies_; /// 敵

	/// Buffer
	ConstantBuffer<MiniMapSize> miniMapBuffer_;
	StructuredBuffer<IconBufferData> friendIconBuffer_;
	StructuredBuffer<IconBufferData> enemyIconBuffer_;
};

