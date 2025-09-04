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

	miniMapSize_ = { 300.0f, 300.0f };
	miniMapPos_ = { 12.0f, 720.0f - 12.0f };
	miniMapPos_.x += miniMapSize_.x / 2.0f;
	miniMapPos_.y -= miniMapSize_.y / 2.0f;

	{
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/MiniMap/MapUI_Direction.png");
		miniMapFrameSprite_.reset(Sprite::Create(textureHandle, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }));
		miniMapFrameSprite_->SetAnchorPoint({ 0.5f, 0.5f });
		Vector2 textureSize = miniMapFrameSprite_->GetTextureSize();
		miniMapFrameSprite_->SetScale(miniMapSize_ / textureSize);
		miniMapFrameSprite_->SetPosition(miniMapPos_);
	}

	{
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/MapUI_Player.png");
		miniMapPlayerIconSprite_.reset(Sprite::Create(textureHandle, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }));
		miniMapPlayerIconSprite_->SetAnchorPoint({ 0.5f, 0.5f });
		Vector2 textureSize = miniMapPlayerIconSprite_->GetTextureSize();
		miniMapPlayerIconSprite_->SetScale(miniMapSize_ / textureSize);
		miniMapPlayerIconSprite_->SetPosition(miniMapPos_);
	}

	/// ミニマップに表示する味方、敵の最大数を設定
	friendIconBuffer_.Create(100, DirectXCommon::GetInstance()->GetDxDevice());
	enemyIconBuffer_.Create(100, DirectXCommon::GetInstance()->GetDxDevice());

	miniMapBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	Vector2 size = miniMapFrameSprite_->GetTextureSize();
	size.x *= miniMapFrameSprite_->transform_.scale.x;
	size.y *= miniMapFrameSprite_->transform_.scale.y;
	miniMapBuffer_.SetMappedData({ miniMapPos_, miniMapSize_.x / 2.0f });

	playerBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	miniMapMatrixBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
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

	/// 仮
	player->SetWorldPosition({-110, 0,0});

	if (player) {
		/// playerの位置、回転を取得
		const Vector3& playerPos = player->GetWorldPosition();
		const Quaternion& playerRot = player->GetQuaternion();
		float playerRotY = -std::atan2(
			2.0f * (playerRot.w * playerRot.y + playerRot.x * playerRot.z),
			1.0f - 2.0f * (playerRot.y * playerRot.y + playerRot.z * playerRot.z)
		);

		miniMapFrameSprite_->transform_.rotate.z = playerRotY;


		/// bufferの更新
		playerBuffer_.SetMappedData({ playerPos });
		Matrix4x4 matMiniMap = MakeAffineMatrix(
			Vector3(miniMapSize_.x, miniMapSize_.y, 1.0f) * 0.5f,
			Vector3(0.0f, 0.0f, playerRotY),
			Vector3(miniMapPos_.x, miniMapPos_.y, 0.0f)
		);
		miniMapMatrixBuffer_.SetMappedData({ matMiniMap, { miniMapPos_, miniMapSize_.x / 2.0f } });



		float scale = 32.0f; 
		/// 味方のアイコン更新
		size_t index = 0;
		for (auto& fd : friends_) {
			Vector3 toPlayerDirection = fd->GetWorldPosition() - playerPos;
			toPlayerDirection.y = 0.0f;
			toPlayerDirection = toPlayerDirection.Normalize();

			/// ある程度離れていたら表示しても意味がないのでスルー
			float distance = (fd->GetWorldPosition() - playerPos).Length();
			if (distance > 320.0f) {
				//continue;
			}

			/// player から見た方向を計算
			Vector2 position = { toPlayerDirection.x * distance, -toPlayerDirection.z * distance };
			/// mini map上の位置に変換
			position += miniMapPos_;

			float rotate = fd->GetTransform().rotation_.y;
			Matrix4x4 matIcon = MakeAffineMatrix(Vector3(scale, scale, 1.0f), Vector3(0.0f, 0.0f, rotate), Vector3(position.x, position.y, 0.0f));

			friendIconBuffer_.SetMappedData(index++, { matIcon });
		}


		/// 敵のアイコン更新
		index = 0;
		for (auto& enemy : enemies_) {
			Vector3 toPlayerDirection = enemy->GetWorldPosition() - playerPos;
			toPlayerDirection.y = 0.0f;
			toPlayerDirection = toPlayerDirection.Normalize();

			/// ある程度離れていたら表示しても意味がないのでスルー
			float distance = (enemy->GetWorldPosition() - playerPos).Length();
			if (distance > 320.0f) {
				//continue;
			}

			/// player から見た方向を計算
			Vector2 position = { toPlayerDirection.x * distance, -toPlayerDirection.z * distance };
			/// mini map上の位置に変換
			position += miniMapPos_;

			float rotate = enemy->GetTransform().rotation_.y;
			Matrix4x4 matIcon = MakeAffineMatrix(Vector3(scale, scale, 1.0f), Vector3(0.0f, 0.0f, rotate), Vector3(position.x, position.y, 0.0f));

			friendIconBuffer_.SetMappedData(index++, { matIcon });
		}

	}

}

void MiniMap::DrawMiniMap() {
	miniMapFrameSprite_->Draw();
	miniMapPlayerIconSprite_->Draw();
}


void MiniMap::RegisterPlayer(BaseObject* _player) {
	playerPtr_ = _player;
}

ConstantBuffer<MiniMapData>& MiniMap::GetMiniMapDataBufferRef() {
	return miniMapBuffer_;
}

StructuredBuffer<IconBufferData>& MiniMap::GetFriendIconBufferRef() {
	return friendIconBuffer_;
}

StructuredBuffer<IconBufferData>& MiniMap::GetEnemyIconBufferRef() {
	return enemyIconBuffer_;
}

ConstantBuffer<PlayerBufferData>& MiniMap::GetPlayerBufferRef() {
	return playerBuffer_;
}

ConstantBuffer<MiniMapMatrix>& MiniMap::GetMiniMapMatrixBufferRef() {
	return miniMapMatrixBuffer_;
}

UINT MiniMap::GetFriendIconCount() const {
	return static_cast<UINT>(friends_.size());
}

UINT MiniMap::GetEnemyIconCount() const {
	return static_cast<UINT>(enemies_.size());
}
