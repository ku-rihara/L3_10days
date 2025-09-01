#include "ModelManager.h"

ModelManager* ModelManager::instance_ = nullptr;
std::map<std::string, std::unique_ptr<Model>> ModelManager::models_;

// シングルトンインスタンス取得
ModelManager* ModelManager::GetInstance() {
    if (!instance_) {
        instance_ = new ModelManager();
    }
    return instance_;
}

void ModelManager::Initialize(DirectXCommon* dxCommon) {
    modelCommon = std::make_unique<ModelCommon>();
    modelCommon->Init(dxCommon);
}

Model* ModelManager::LoadModel(const std::string& modelName) {
    // 読み込み済みモデルを検索
    if (models_.contains(modelName)) {
        // 読み込み済みなら早期リターン
        return nullptr;
    }
    Model* model = new Model();
    model->CreateModel(modelName);
    // モデルをmapコンテナに格納する
    models_.insert(std::make_pair(modelName, std::move(model)));

    return model;
}


Model* ModelManager::FindModel(const std::string& modelName) {
    // 読み込み済モデルを検索
    if (models_.contains(modelName)) {
        // 読み込み済モデルをreturn 
        return models_.at(modelName).get();
    }
    // ファイル名一致なし
    return nullptr;
}

// 終了
void ModelManager::Finalize() {
    models_.clear(); // モデルデータを解放
    modelCommon.reset(); // ModelCommonを解放
    delete instance_;
    instance_ = nullptr; // インスタンスをnullptrにする
}
