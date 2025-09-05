#include "CameraRendition.h"
#include "Frame/Frame.h"
#include "GameCamera.h"
#include "Shake.h"

void CameraRendition::Init() {
  
    shakePlayer_ = std::make_unique<ShakePlayer>();
    cameraAnimation_ = std::make_unique<CameraAnimation>();

    shakePlayer_->Init();
}

void CameraRendition::Update() {
  
    shakePlayer_->Update(Frame::DeltaTime());
    cameraAnimation_->Update(Frame::DeltaTimeRate());
}

 // play呼び出し
void CameraRendition::AnimationPlay(const std::string& filename) {
    cameraAnimation_->Play(filename);
}
void CameraRendition::ShakePlay(const std::string& filename) {
    shakePlayer_->Play(filename);
}

 void CameraRendition::SetViewProjection(ViewProjection* viewProjection) {
    cameraAnimation_->Init(viewProjection);
 }

 void CameraRendition::Edit() {
     shakePlayer_->EditorUpdate();
     cameraAnimation_->EditorUpdate();
   }