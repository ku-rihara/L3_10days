#pragma once

class GameClearEffect {
public:
	GameClearEffect();
	~GameClearEffect();

	void Init();
	void Update();
	void Draw();
	
	void Start();
	bool GetIsEnd();
private:
	bool isStart_ = false;
	bool isEnd_ = false;
	float timer_ = 0.0f;
	const float effectTime_ = 2.0f;

	/// 演出
};

