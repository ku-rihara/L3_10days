#pragma once

/// std
#include <memory>

/// engine
#include "2d/Sprite.h"

/// base
#include "GameOptionItem.h"

/// ////////////////////////////////////////////////////////////////////////////////////
/// @brief 操作設定項目
/// ////////////////////////////////////////////////////////////////////////////////////
class OperationOptionItem : public IGameOptionItem {
public:
	/// ==================================
	/// public : methods
	/// ==================================

	OperationOptionItem() = default;
	~OperationOptionItem() override = default;

	void Init() override;
	void Update(size_t _currentIndex) override;
	void Draw() override;

private:
	/// ==================================
	/// private : objects
	/// ==================================

	/// コントローラーのtexture
	using USprite = std::unique_ptr<Sprite>;

	USprite controllerSprite_ = nullptr;
	USprite frame_;
};

