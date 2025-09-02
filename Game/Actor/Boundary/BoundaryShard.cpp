#include "BoundaryShard.h"

/// std
#include <filesystem>
#include <cassert>

/// assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

/// engine
#include "3d/ModelManager.h"

/// game
#include "Boundary.h"

BoundaryShard::BoundaryShard() {}
BoundaryShard::~BoundaryShard() {}

void BoundaryShard::Init() {
	/// ここで使用するモデルの読み込み
	LoadShardModel("./Resources/Model/BoundaryShard/BoundaryShard.obj");

	breakableTransformBuffer_.Create(
		static_cast<uint32_t>(Boundary::GetInstance()->GetMaxHoleCount()),
		DirectXCommon::GetInstance()->GetDxDevice()
	);
}

void BoundaryShard::Update() {

	for (auto& breakable : breakables_) {
		/// 罅のmaxLifeとcurrentLifeを見てstageを決定
		float lifeRatio = breakable.currentLife / breakable.maxLife;

		/// 4段階で設定 (0:無傷, 1:軽傷, 2:重傷, 3:破片に分かれる)
		if (lifeRatio > 0.66f) {
			breakable.stage = 0;
		} else if (lifeRatio > 0.33f) {
			breakable.stage = 1;
		} else if (lifeRatio > 0.0f) {
			breakable.stage = 2;
		} else {
			breakable.stage = 3;
		}
	}

}

void BoundaryShard::LoadShardModel(const std::string& _filepath) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(_filepath, aiProcess_FlipWindingOrder | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (!scene) {
		return; ///< 読み込み失敗
	}

	loadedShards_.clear();

	/// mesh 解析
	for (uint32_t meshIndex = 0u; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		/// sceneのデータを使ってMeshを作成する
		std::vector<ShardVertex> vertices;
		std::vector<uint32_t> indices;
		vertices.reserve(mesh->mNumVertices);
		indices.reserve(mesh->mNumFaces * 3);

		/// vertex 解析
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			ShardVertex&& vertex = {
				Vector4(-mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f),
				Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y),
				Vector3(-mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
			};
			vertices.push_back(vertex);
		}

		/// index 解析
		for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			for (uint32_t j = 0; j < face.mNumIndices; ++j) {
				indices.push_back(face.mIndices[j]);
			}
		}

		/// mesh dataを作成
		Shard shard;
		shard.vertexBuffer.Create(static_cast<uint32_t>(vertices.size()), DirectXCommon::GetInstance()->GetDxDevice());
		shard.indexBuffer.Create(static_cast<uint32_t>(indices.size()), DirectXCommon::GetInstance()->GetDxDevice());

		shard.vertexBuffer.SetVertices(vertices);
		shard.indexBuffer.SetIndices(indices);

		shard.vertexBuffer.Map();
		shard.indexBuffer.Map();

		loadedShards_.push_back(std::move(shard));
	}
}

const std::vector<Breakable>& BoundaryShard::GetBreakables() const {
	return breakables_;
}

std::vector<Breakable>& BoundaryShard::GetBreakablesRef() {
	return breakables_;
}

const std::vector<Shard>& BoundaryShard::GetLoadedShards() const {
	return loadedShards_;
}

void BoundaryShard::AddBreakable(const Vector3& _position, float _damage) {

	/// すでにある罅と近かったら追加しないで近い罅のlifeを減らす
	bool isNearBreakable = false;
	for (size_t i = 0; i < breakables_.size(); i++) {
		Breakable& other = breakables_[i];
		float distance = (other.position - _position).Length();
		if (distance < 100.0f) {
			other.currentLife -= _damage;
			other.radius = std::min(other.radius + _damage, 200.0f);
			isNearBreakable = true;
		}
	}

	/// 近くに罅があったときの処理をしたかチェック、したならreturn
	if (isNearBreakable) {
		return;
	}

	/// 罅の追加
	Breakable breakable;
	breakable.position = _position;
	breakable.maxLife = 320.0f;
	breakable.currentLife = breakable.maxLife;
	breakable.stage = 0;
	breakable.radius = _damage;
	breakable.shards = loadedShards_;
	breakable.transformBuffer.Create(
		static_cast<uint32_t>(breakable.shards.size()),
		DirectXCommon::GetInstance()->GetDxDevice()
	);

	breakables_.push_back(std::move(breakable));

}

