#include "BoundaryShard.h"

/// std
#include <filesystem>
#include <cassert>
#include <numbers>

/// assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

/// engine
#include "3d/ModelManager.h"
#include "random.h"
#include "Frame/Frame.h"

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

	breakableBuffer_.Create(
		static_cast<uint32_t>(Boundary::GetInstance()->GetMaxHoleCount()),
		DirectXCommon::GetInstance()->GetDxDevice()
	);

	instanceCountBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
}

void BoundaryShard::Update() {

	for (auto& breakable : breakables_) {
		breakable.frameTime += Frame::DeltaTime();

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

		/// stageに応じて破片のtransformを更新
		float baseOffset = 0.02f;
		float stageOffset = 0.001f;

		for (size_t i = 0; i < breakable.shards.size(); i++) {
			Shard& shard = breakable.shards[i];

			if (shard.phase == 0.0f) {
				shard.phase = Random::Range(0.0f, std::numbers::pi_v<float> *2.0f);
			}

			shard.transform.translate = shard.initPos + shard.normal * (baseOffset + stageOffset * (breakable.radius));
			{
				float amplitude = 0.05f * (breakable.radius * 0.1f); // 揺れる大きさ
				float frequency = 2.0f; // 揺れる速さ

				float wave1 = sin(breakable.frameTime * frequency + shard.phase);
				float wave2 = cos(breakable.frameTime * (frequency * 0.7f) + shard.phase);

				shard.offsetPos = shard.normal * (wave1 * amplitude) * (wave2 * amplitude * 0.5f);
			}

			/// 回転
			shard.transform.rotate = shard.initRotate * (shard.randomSmallRotation * (breakable.radius * 0.1f));
			{
				float amplitudeRot = 5.0f * (std::numbers::pi_v<float> / 180.0f); // 揺れる角度（ラジアン）
				float frequencyRot = 1.5f; // 揺れる速さ

				// shardごとにランダム位相を持っていると自然
				float waveRot = std::sin(breakable.frameTime * frequencyRot + shard.phase);

				// 回転オフセット（ラジアン）
				shard.offsetRotate = Vector3(0.0f, 0.0f, waveRot * amplitudeRot);
			}
		}
	}


	/// bufferに詰める
	Boundary* boundary = Boundary::GetInstance();
	for (size_t i = 0; i < boundary->GetMaxHoleCount(); i++) {
		if (i < breakables_.size()) {
			auto& breakable = breakables_[i];
			breakableBuffer_.SetMappedData(i, { breakable.position, breakable.radius });
		} else {
			breakableBuffer_.SetMappedData(i, {});
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
		Vector3 averageNormal = {};
		Vector3 averagePos = {};
		for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
			ShardVertex&& vertex = {
				Vector4(-mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1.0f),
				Vector2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y),
				Vector3(-mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
			};

			averageNormal += Vector3(-mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			averagePos += Vector3(-mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

			vertices.push_back(vertex);
		}

		averagePos = averagePos / static_cast<float>(mesh->mNumVertices);
		for (auto& v : vertices) {
			v.position.x -= averagePos.x;
			v.position.y -= averagePos.y;
			v.position.z -= averagePos.z;
		}

		averageNormal = (averagePos.Normalize() + Vector3(0.0f, Random::Range(-1.0f, 1.0f), 0.0f)).Normalize();

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

		shard.initPos = averagePos;
		shard.initRotate = {
			Random::Range(-1.0f, 1.0f),
			Random::Range(-1.0f, 1.0f),
			Random::Range(-1.0f, 1.0f)
		};
		shard.normal = averageNormal;
		shard.randomSmallRotation = Random::Range(0.0f, 1.0f);

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

StructuredBuffer<BreakableBufferData>& BoundaryShard::GetBreakableBufferRef() {
	return breakableBuffer_;
}

void BoundaryShard::AddBreakable(const Vector3& _position, float _damage) {

	/// すでにある罅と近かったら追加しないで近い罅のlifeを減らす
	bool isNearBreakable = false;
	for (size_t i = 0; i < breakables_.size(); i++) {
		Breakable& other = breakables_[i];
		float distance = (other.position - _position).Length();
		if (distance < other.radius) {
			other.currentLife -= _damage;
			other.radius = std::min(other.radius + _damage, 50.0f);
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
	breakable.maxLife = 100.0f;
	breakable.currentLife = breakable.maxLife;
	breakable.stage = 0;
	breakable.radius = _damage;
	breakable.shards = loadedShards_;

	breakables_.push_back(std::move(breakable));
}

