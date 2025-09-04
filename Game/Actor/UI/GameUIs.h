#pragma once

#include <memory>

/// ui
#include "PlayerUI/PlayerSpeedUI.h"
#include "PlayerUI/PlayerAltUI.h"

class GameUIs {
public:
	GameUIs()  = default;
	~GameUIs() = default;
	void Init();
	void Update(class Player* _player);
	void Draw();

private:

	std::unique_ptr<PlayerSpeedUI> playerSpeedUI_;
	std::unique_ptr<PlayerAltUI> playerAltUI_;

};

