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
	TextureManager::GetInstance()->LoadTexture("./resources/Texture/MiniMap/RadarIconEnemy.png");
	TextureManager::GetInstance()->LoadTexture("./resources/Texture/uvChecker.png");

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
	playerMissile_.Create(12, DirectXCommon::GetInstance()->GetDxDevice());

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
	if (player) {
		/// playerの位置、回転を取得
		const Vector3& playerPos = player->GetWorldPosition();
		const Quaternion& playerRot = player->GetBaseQuaternion();
		float playerRotY = -std::atan2(
			2.0f * (playerRot.w * playerRot.y + playerRot.x * playerRot.z),
			1.0f - 2.0f * (playerRot.y * playerRot.y + playerRot.z * playerRot.z)
		);

		miniMapFrameSprite_->transform_.rotate.z = playerRotY;
		matPlayerRotY_ = MakeRotateYMatrix(playerRotY);


		/// bufferの更新
		playerBuffer_.SetMappedData({ playerPos });
		Matrix4x4 matMiniMap = MakeAffineMatrix(
			Vector3(miniMapSize_.x, miniMapSize_.y, 1.0f) * 0.5f,
			Vector3(0.0f, 0.0f, playerRotY),
			Vector3(miniMapPos_.x, miniMapPos_.y, 0.0f)
		);
		miniMapMatrixBuffer_.SetMappedData({ matMiniMap, { miniMapPos_, miniMapSize_.x / 2.0f } });



		float mapScale = 0.1f; /// マップの縮尺
		float scale = 32.0f;
		/// 味方のアイコン更新
		size_t index = 0;
		for (auto& fd : friends_) {
			Vector3 relativePos = fd->GetWorldPosition() - playerPos;
			Vector3 localPos = TransformMatrix(relativePos, matPlayerRotY_);
			localPos *= mapScale;
			Vector2 mapPos = { miniMapPos_.x + localPos.x, miniMapPos_.y - localPos.z };
			Matrix4x4 matIcon = MakeAffineMatrix(
				Vector3(scale, scale, 1.0f),
				Vector3(0.0f, 0.0f, fd->GetTransform().rotation_.y),
				Vector3(mapPos.x, mapPos.y, 0.0f)
			);
			friendIconBuffer_.SetMappedData(index++, { matIcon });
		}


		/// 敵のアイコン更新
		index = 0;
		for (auto& enemy : enemies_) {
			Vector3 relativePos = enemy->GetWorldPosition() - playerPos;
			Vector3 localPos = TransformMatrix(relativePos, matPlayerRotY_);
			localPos *= mapScale;
			Vector2 mapPos = { miniMapPos_.x + localPos.x, miniMapPos_.y - localPos.z };
			Matrix4x4 matIcon = MakeAffineMatrix(
				Vector3(scale, scale, 1.0f),
				Vector3(0.0f, 0.0f, enemy->GetTransform().rotation_.y),
				Vector3(mapPos.x, mapPos.y, 0.0f)
			);
			enemyIconBuffer_.SetMappedData(index++, { matIcon });
		}

		/// ミサイルのアイコン更新
		index = 0;
		for (const auto& missile : player->GetBulletShooter()->GetActiveMissiles()) {
			Vector3 relativePos = missile->GetWorldPosition() - playerPos;
			Vector3 localPos = TransformMatrix(relativePos, matPlayerRotY_);
			localPos *= mapScale;
			Vector2 mapPos = { miniMapPos_.x + localPos.x, miniMapPos_.y - localPos.z };
			Matrix4x4 matIcon = MakeAffineMatrix(
				Vector3(scale, scale, 1.0f),
				Vector3(0.0f, 0.0f, missile->GetTransform().rotation_.y),
				Vector3(mapPos.x, mapPos.y, 0.0f)
			);
			playerMissile_.SetMappedData(index++, { matIcon });
		}

		missileCount_ = static_cast<UINT>(player->GetBulletShooter()->GetActiveMissiles().size());
	}

}

void MiniMap::DrawMiniMapFrame() {
	miniMapFrameSprite_->Draw();
}

void MiniMap::DrawMiniMapPlayerIcon() {
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

StructuredBuffer<IconBufferData>& MiniMap::GetPlayerMissileBufferRef() {
	return playerMissile_;
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

UINT MiniMap::GetPlayerMissileCount() const {
	return missileCount_;
}

Vector2 MiniMap::ToVector2(const Vector3& _vec3) {
	return Vector2(_vec3.x, _vec3.z);
}

Vector2 MiniMap::ApplyRotation(const Vector3& _point) {
	/// プレイヤーの回転を考慮して回転させる
	return ToVector2(TransformMatrix(_point, matPlayerRotY_));
}

