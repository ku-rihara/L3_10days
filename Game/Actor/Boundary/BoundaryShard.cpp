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
	//ModelManager::GetInstance()->LoadModel("BoundaryShard.obj");
	LoadShardModel("./Resources/Model/BoundaryShard/BoundaryShard.obj");
}

void BoundaryShard::Update() {

	/// 境界のポインタを取得
	Boundary* boundary = Boundary::GetInstance();
	if (!boundary) {
		return;
	}

	/// 境界の罅の数を取得
	auto& cracks = boundary->GetCracksRef();
	if(cracks.size() == 0) {
		return;
	}

	/// 罅の数だけ破片を生成
	for(auto itr = cracks.begin(); itr != cracks.end(); ++itr) {
		//const Crack& crack = *itr;
	}

}

void BoundaryShard::LoadShardModel(const std::string& _filepath) {

	/// ファイルの拡張子を取得
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

