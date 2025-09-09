#include"SceneFactory.h"
#include"Scene/TitleScene.h"
#include"Scene/GameScene.h"
#include"Scene/EditorScene.h"
#include"Scene/GameClearScene.h"
#include"Scene/GameOverScene.h"
#include"Scene/TutorialScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName) {
	// 次のシーンを生成
	BaseScene* newScene = nullptr;

	if (sceneName == "TITLE") {
		newScene = new TitleScene();
	} else if (sceneName == "GAMEPLAY") {
		newScene = new GameScene();
	} else if (sceneName == "EDITOR") {
		newScene = new EditorScene();
	} else if (sceneName == "GAMECLEAR") {
		newScene = new GameClearScene();
	} else if (sceneName == "GAMEOVER") {
		newScene = new GameOverScene();
	} else if (sceneName == "TUTORIAL") {
        newScene = new TutorialScene();
    }

	return newScene;
}