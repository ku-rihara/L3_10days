#pragma once

#include "NPC.h"
#include <memory>
#include <2d/Sprite.h>

class PlayerNPC final :
	public NPC {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	PlayerNPC();
	~PlayerNPC()override;

	void Init()override;
	void Update()override;

	void SpriteUpdate(const ViewProjection& viewPro);
    void DrawSprite();

private:
    std::unique_ptr<Sprite> bossReticle_;
    float spriteRotation_;
    bool isDraw_ = false;
};