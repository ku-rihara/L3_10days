#include "MiniMap.h"

/// engine
#include "base/TextureManager.h"

/// game
#include "Actor/Player/Player.h"
#include "Actor/Station/Base/BaseStation.h"
#include "Actor/NPC/NPC.h"

void MiniMap::Init(BaseStation* _ally, BaseStation* _enemy) {
	pAllyStation_ = _ally;
	pEnemyStation_ = _enemy;


	TextureManager::GetInstance()->LoadTexture("./resources/Texture/MiniMap/Icon.png");

	uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/MapUI_Direction.png");
	miniMapFrameSprite_.reset(Sprite::Create(textureHandle, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }));
	miniMapFrameSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	miniMapFrameSprite_->SetScale({ 0.2f, 0.2f });
	miniMapFrameSprite_->SetPosition({ 120.0f, 600.0f });

	/// ミニマップに表示する味方、敵の最大数を設定
	friendIconBuffer_.Create(100, DirectXCommon::GetInstance()->GetDxDevice());
	enemyIconBuffer_.Create(100, DirectXCommon::GetInstance()->GetDxDevice());

	miniMapBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	Vector2 pos = { 120.0f, 600.0f };
	Vector2 size = miniMapFrameSprite_->GetTextureSize();
	size.x *= miniMapFrameSprite_->transform_.scale.x;
	size.y *= miniMapFrameSprite_->transform_.scale.y;
	miniMapBuffer_.SetMappedData({ pos - size, pos + size });
}


void MiniMap::Update() {

	/// friends_, enemies_を更新
	friends_.clear();
	enemies_.clear();

	for (auto& npc : pAllyStation_->GetLiveNpcs()) {
		friends_.push_back(npc);
	}

	for (auto& npc : pEnemyStation_->GetLiveNpcs()) {
		enemies_.push_back(npc);
	}


	/// ICONに積める
	Player* player = dynamic_cast<Player*>(playerPtr_);
	if (player) {
		/// playerの位置、回転を取得
		const Vector3& playerPos = player->GetPosition();
		float playerRotY = player->GetRotation().y;

		/// 味方のアイコン更新
		size_t index = 0;
		for (auto& fd : friends_) {
			Vector3 toPlayerDirection = fd->GetWorldPosition() - playerPos;
			toPlayerDirection.y = 0.0f;
			toPlayerDirection = toPlayerDirection.Normalize();

			/// ある程度離れていたら表示しても意味がないのでスルー
			float distance = (fd->GetWorldPosition() - playerPos).Length();
			if (distance > 1000.0f) {
				continue;
			}

			/// ミニマップ上の位置を計算
			Vector2 miniMapPos = { toPlayerDirection.x * distance, toPlayerDirection.z * distance };
			Matrix4x4 matMiniMap = MakeAffineMatrix(Vector3(5.0f, 5.0f, 1.0f), Vector3(0.0f, -playerRotY, 0.0f), Vector3(miniMapPos.x, miniMapPos.y, 0.0f));

			friendIconBuffer_.SetMappedData(index++, { matMiniMap });
		}


		index = 0;
		for (auto& enemy : enemies_) {
			Vector3 toPlayerDirection = enemy->GetWorldPosition() - playerPos;
			toPlayerDirection.y = 0.0f;
			toPlayerDirection = toPlayerDirection.Normalize();

			/// ある程度離れていたら表示しても意味がないのでスルー
			float distance = (enemy->GetWorldPosition() - playerPos).Length();
			if (distance > 1000.0f) {
				continue;
			}

			/// ミニマップ上の位置を計算
			Vector2 miniMapPos = { toPlayerDirection.x * distance, toPlayerDirection.z * distance };
			Matrix4x4 matMiniMap = MakeAffineMatrix(Vector3(5.0f, 5.0f, 1.0f), Vector3(0.0f, -playerRotY, 0.0f), Vector3(miniMapPos.x, -miniMapPos.y, 0.0f));

			enemyIconBuffer_.SetMappedData(index++, { matMiniMap });
		}
	}

}

void MiniMap::DrawMiniMap() {
	miniMapFrameSprite_->Draw();
}


void MiniMap::RegisterPlayer(BaseObject* _player) {
	playerPtr_ = _player;
}

ConstantBuffer<MiniMapSize>& MiniMap::GetMiniMapSizeBuffer() {
	return miniMapBuffer_;
}

StructuredBuffer<IconBufferData>& MiniMap::GetFriendIconBuffer() {
	return friendIconBuffer_;
}

StructuredBuffer<IconBufferData>& MiniMap::GetEnemyIconBuffer() {
	return enemyIconBuffer_;
}

UINT MiniMap::GetFriendIconCount() const {
	return static_cast<UINT>(friends_.size());
}

UINT MiniMap::GetEnemyIconCount() const {
	return static_cast<UINT>(enemies_.size());
}
