#pragma once

#include <SNX/Core/Scene/Scene.h>

#include <Games/Tank3D/TankGame3D.h>

class TankScene final : public Scene {
public:
	TankScene(SceneManager& sceneManager, SceneContext& context) noexcept;

protected:
	void OnLoad() override;
	void OnUnload() override;

	void OnUpdate() override;

	void OnRenderWorld() override;
	void OnRenderUI() override;

	[[nodiscard]]
	bool ShouldUpdateGameObjects() const noexcept override;

	[[nodiscard]]
	std::array<float, 4> OnGetClearColor() const noexcept override;

private:
	enum class State {
		Playing,
		Paused,
		GameOver
	};

	void RestartGame();

	void PauseGame();
	void ResumeGame();

	void EnterGameOver();

	void DrawGameplayUI();
	void DrawPauseUI();
	void DrawGameOverUI();

private:
	TankGame3D m_tankGame;

	State m_state = State::Playing;
};