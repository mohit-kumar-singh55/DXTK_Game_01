#pragma once

#include <SNX/Core/Scene/Scene.h>

#include <Games/Shooter2D/ShooterGame2D.h>

class ShooterScene final : public Scene {
public:
	ShooterScene(SceneManager& sceneManager, SceneContext& context) noexcept;

protected:
	void OnLoad() override;
	void OnUnload() override;

	void OnUpdate() override;
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

	void DrawGameplayUI();
	void DrawPauseUI();
	void DrawGameOverUI();

private:
	ShooterGame2D m_shooterGame;

	State m_state = State::Playing;
};