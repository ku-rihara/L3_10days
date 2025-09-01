#include "Object3d.h"
#include "ModelManager.h"
#include "Object3DRegistry.h"
#include "Pipeline/Object3DPiprline.h"

Object3d::~Object3d() {
    if (Object3DRegistry::GetInstance()) {
        Object3DRegistry::GetInstance()->UnregisterObject(this);
    }
}
///============================================================
/// モデル作成
///============================================================
Object3d* Object3d::CreateModel(const std::string& instanceName) {
    // 新しいModelインスタンスを作成
    std::unique_ptr<Object3d> object3d = std::make_unique<Object3d>();
    ModelManager::GetInstance()->LoadModel(instanceName);
    object3d->transform_.Init();
    object3d->SetModel(instanceName);
    object3d->CreateWVPResource();
    object3d->CreateShadowMap();
    object3d->CreateMaterialResource();

    Object3DRegistry::GetInstance()->RegisterObject(object3d.get());

    return object3d.release();
}

///============================================================
/// 初期化
///============================================================
void Object3d::Init() {
}

void Object3d::Update() {
    transform_.UpdateMatrix();
    material_.materialData_->color = objColor_.GetColor();
}

///============================================================
/// 描画
///============================================================
void Object3d::Draw(const ViewProjection& viewProjection) {
    if (!model_||!isDraw_) {
        return;
    }

    UpdateWVPData(viewProjection);

    Object3DPiprline::GetInstance()->PreBlendSet(DirectXCommon::GetInstance()->GetCommandList(), blendMode);
    model_->Draw(wvpResource_, *shadowMap_, material_);
}

void Object3d::ShadowDraw(const ViewProjection& viewProjection) {
    if (!model_ || !isShadow_ ||!isDraw_) {
        return;
    }

    UpdateWVPData(viewProjection);

    model_->DrawForShadowMap(wvpResource_, *shadowMap_);
}

///============================================================
/// デバッグ表示
///============================================================

void Object3d::DebugImgui() {
    BaseObject3d::DebugImgui();
}

///============================================================
/// WVP更新
///============================================================
void Object3d::UpdateWVPData(const ViewProjection& viewProjection) {
    BaseObject3d::UpdateWVPData(viewProjection);
}

///============================================================
/// マテリアルリソース作成
///============================================================
void Object3d::CreateMaterialResource() {
    BaseObject3d::CreateMaterialResource();
}

///============================================================
/// WVPリソース作成
///============================================================
void Object3d::CreateWVPResource() {
    BaseObject3d::CreateWVPResource();
}

void Object3d::CreateShadowMap() {
    BaseObject3d::CreateShadowMap();
}
