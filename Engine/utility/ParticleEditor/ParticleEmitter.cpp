#include "ParticleEmitter.h"
#include "Frame/Frame.h"
#include "ParticleManager.h"
#include <imgui.h>


///=================================================================================
/// パーティクル作成
///=================================================================================
ParticleEmitter* ParticleEmitter::CreateParticle(const std::string& name, const std::string& modelFilePath, const int32_t& maxnum) {

    auto emitter           = std::make_unique<ParticleEmitter>();
    emitter->particleName_ = name;

    ParticleManager::GetInstance()->CreateParticleGroup(emitter->particleName_, modelFilePath, maxnum);
    emitter->Init();
    return emitter.release();
}

///=================================================================================
/// パーティクル作成(Primitive)
///=================================================================================
ParticleEmitter* ParticleEmitter::CreateParticlePrimitive(const std::string& name, const PrimitiveType& primitiveType, const int32_t& maxnum) {

    auto emitter           = std::make_unique<ParticleEmitter>();
    emitter->particleName_ = name;

    ParticleManager::GetInstance()->CreatePrimitiveParticle(emitter->particleName_, primitiveType, maxnum);
    emitter->Init();
    return emitter.release();
}

/// =================================================================================
/// 初期化
/// =================================================================================
void ParticleEmitter::Init() {

    ParticleParameter::Init();

    // レールマネージャー
    railManager_ = std::make_unique<RailManager>();
    railManager_->Init(particleName_ + "Emit");

    /// 発生位置可視化オブジェ
  /*  obj3d_.reset(Object3d::CreateModel("DebugCube.obj"));*/
    debugLine_.Init(24); 
    emitBoxTransform_.Init();
}

///=================================================================================
/// エミット
///=================================================================================
void ParticleEmitter::Emit() {

    // 　発生座標のパターン切り替え
    if (isMoveForRail_) {
        parameters_.emitPos = railManager_->GetWorldTransform().GetWorldPos();

        if (!isStartRailMove_) {
            return;
        }

    } else {
        parameters_.emitPos = parameters_.emitPos;
    }

    currentTime_ += Frame::DeltaTime(); // 時間加算

    if (currentTime_ >= intervalTime_ || groupParamaters_.isShot) { // 　間隔ごとに発動

        ParticleManager::GetInstance()->Emit(
            particleName_, parameters_, groupParamaters_, particleCount_);
        currentTime_ = 0.0f;
    }
}

void ParticleEmitter::Update() {

    RailMoveUpdate();
    UpdateEmitTransform();

    /* SetValues();*/
}

void ParticleEmitter::RailMoveUpdate() {
    // レール更新
    if (!isStartRailMove_) {
        return;
    }
    railManager_->Update(moveSpeed_);

    if (railManager_->GetRailMoveTime() < 1.0f) {
        return;
    }

    if (!isRailRoop_) {
     isStartRailMove_ = false;
    }
}

void ParticleEmitter::EditorUpdate() {
    ParticleParameter::EditorUpdate();
}

void ParticleEmitter::StartRailEmit() {

    isStartRailMove_ = true;
    railManager_->SetRailMoveTime(0.0f);
    railManager_->SetIsRoop(isRailRoop_);
}

void ParticleEmitter::UpdateEmitTransform() {
    emitBoxTransform_.translation_ = parameters_.emitPos;
    // スケールを範囲の大きさで設定
    emitBoxTransform_.scale_ = {
        parameters_.positionDist.max.x - parameters_.positionDist.min.x,
        parameters_.positionDist.max.y - parameters_.positionDist.min.y,
        parameters_.positionDist.max.z - parameters_.positionDist.min.z};
   
    emitBoxTransform_.UpdateMatrix();
}

void ParticleEmitter::RailDraw(const ViewProjection& viewProjection) {
    viewProjection;
    /*railManager_->RailDraw(viewProjection);*/
}
void ParticleEmitter::DebugDraw(const ViewProjection& viewProjection) {
    viewProjection;
#ifdef _DEBUG

    if (isMoveForRail_) { // レールに沿うエミット位置
        railManager_->Draw(viewProjection, emitBoxTransform_.scale_);

    } else { // レールに沿わないエミット位置
        debugLine_.DrawCubeWireframe(emitBoxTransform_.GetWorldPos(), emitBoxTransform_.scale_,Vector4::kWHITE());
        debugLine_.Draw(viewProjection);
    }
#endif // _DEBUG
}

///=================================================================================
/// テクスチャ切り替え
///=================================================================================
void ParticleEmitter::SetTextureHandle(const uint32_t& handle) {
    ParticleManager::GetInstance()->SetTextureHandle(particleName_, handle);
}

/// =======================================================================================
/// setter method
/// ========================================================================================
void ParticleEmitter::SetParentBasePos(WorldTransform* parent) {
    emitBoxTransform_.parent_ = parent;
}

void ParticleEmitter::SetTargetRotate(const Vector3& _targetRotate) {
	parameters_.targetRotate = _targetRotate;
}

void ParticleEmitter::SetBlendMode(const BlendMode& blendmode) {
    groupParamaters_.blendMode = blendmode;
}

void ParticleEmitter::SetBillBordType(const BillboardType& billboardType) {
    groupParamaters_.billBordType = billboardType;
}

void ParticleEmitter::SetParentTransform(const WorldTransform* transform) {
    parameters_.parentTransform = transform;
}

void ParticleEmitter::SetFollowingPos(const Vector3* pos) {
    parameters_.followingPos_ = pos;
}


void ParticleEmitter::SetParentJoint(const Object3DAnimation* modelAnimation, const std::string& name) {
    parameters_.jointParent.animation = modelAnimation;
    parameters_.jointParent.name           = name;
 }