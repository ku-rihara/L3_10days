#include "MiniMap.h"

/// engine
#include "base/TextureManager.h"

/// game
#include "Actor/Player/Player.h"

void MiniMap::Init() {

	uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/MapUI_Direction.png");
	miniMapFrameSprite_.reset(Sprite::Create(textureHandle, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }));
	miniMapFrameSprite_->SetAnchorPoint({ 0.5f, 0.5f });
	miniMapFrameSprite_->SetScale({ 0.2f, 0.2f });
	miniMapFrameSprite_->SetPosition({ 120.0f, 600.0f });

	/// ミニマップに表示する味方、敵の最大数を設定
	friendIconBuffer_.Create(100, DirectXCommon::GetInstance()->GetDxDevice());
	enemyIconBuffer_.Create(100, DirectXCommon::GetInstance()->GetDxDevice());

}

void MiniMap::Update() {
	Player* player = dynamic_cast<Player*>(playerPtr_);
	if (player) {
		/// playerの位置、回転を取得
		const Vector3& playerPos = player->GetPosition();
		float playerRotY = player->GetRotation().y;

		/// 味方のアイコン更新
		size_t index = 0;
		for (auto& fd : friends_) {
			Vector3 toPlayerDirection = fd->transform_.GetWorldPos() - playerPos;
			toPlayerDirection.y = 0.0f;
			toPlayerDirection = toPlayerDirection.Normalize();

			/// ある程度離れていたら表示しても意味がないのでスルー
			float distance = (fd->transform_.GetWorldPos() - playerPos).Length();
			if(distance > 120.0f) {
				continue;
			}

			/// ミニマップ上の位置を計算
			Vector2 miniMapPos = { toPlayerDirection.x * distance, toPlayerDirection.z * distance };
			Matrix4x4 matMiniMap = MakeAffineMatrix(Vector3(5.0f, 5.0f, 1.0f), Vector3(0.0f, -playerRotY, 0.0f), Vector3(miniMapPos.x, miniMapPos.y, 0.0f));

			friendIconBuffer_.SetMappedData(index++, { matMiniMap });
		}

	}
}

void MiniMap::DrawMiniMap() {
	miniMapFrameSprite_->Draw();
}


void MiniMap::RegisterPlayer(BaseObject3d* _player) {
	playerPtr_ = _player;
}

void MiniMap::AddFriend(BaseObject3d* _friend) {
	friends_.emplace_back(_friend);
}

void MiniMap::RemoveFriend(BaseObject3d* _friend) {
	for (auto itr = friends_.begin(); itr != friends_.end(); ) {
		if (*itr == _friend) {
			itr = friends_.erase(itr);
		} else {
			++itr;
		}
	}
}

void MiniMap::AddEnemy(BaseObject3d* _enemy) {
	enemies_.emplace_back(_enemy);
}

void MiniMap::RemoveEnemy(BaseObject3d* _enemy) {
	for (auto itr = enemies_.begin(); itr != enemies_.end(); ) {
		if (*itr == _enemy) {
			itr = enemies_.erase(itr);
		} else {
			++itr;
		}
	}
}

ConstantBuffer<MiniMapBufferData>& MiniMap::GetMiniMapBuffer() {
	return miniMapBuffer_;
}

StructuredBuffer<IconBufferData>& MiniMap::GetFriendIconBuffer() {
	return friendIconBuffer_;
}

StructuredBuffer<IconBufferData>& MiniMap::GetEnemyIconBuffer() {
	return enemyIconBuffer_;
}
