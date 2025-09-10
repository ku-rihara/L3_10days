#pragma once


/// @brief ゲームのスコアを持つ
class GameScore final {
	GameScore();
	~GameScore();
public:
	/// instance
	static GameScore* GetInstance();

	/// reset
	void ScoreReset();

	/// setter
	void AddBreakEnemyCount(int count = 1);
	void SetClearTime(float time);

	/// getter
	int GetBreakEnemyCount() const;
	float GetClearTime() const;

private:
	int breakEnemyCount_ = 0;  //< 撃破した敵の数
	float clearTime_ = 0.0f;   //< クリアタイム
};

