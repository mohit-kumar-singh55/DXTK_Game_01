#include "ShooterScene.h"

#include "SceneIds.h"

#include <SNX/Audio/AudioManager.h>
#include <SNX/Core/Time.h>
#include <SNX/Graphics/DeviceResources.h>
#include <SNX/Input/InputManager.h>

#include <DirectXColors.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

#include <string>

ShooterScene::ShooterScene(SceneManager& sceneManager, SceneContext& context) noexcept :
	Scene(sceneManager, context) {}

void ShooterScene::OnLoad() {
	auto& context = GetContext();

	auto& deviceResources = context.deviceResources;

	m_shooterGame.Initialize(
		deviceResources.GetDevice(),
		deviceResources.GetWidth(),
		deviceResources.GetHeight()
	);

	m_state = State::Playing;

	auto& input = InputManager::Get();

	input.SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
	input.Reset();
}

void ShooterScene::OnUnload() {
	m_shooterGame.Clear();
}

void ShooterScene::OnUpdate() {
	auto& input = InputManager::Get();

	switch (m_state) {
	case State::Playing: {
		if (input.IsKeyPressed(DirectX::Keyboard::Escape)) {
			PauseGame();
			return;
		}

		m_shooterGame.Update(Time::DeltaTime(), GetContext().audioManager);

		if (m_shooterGame.IsGameOver())
			m_state = State::GameOver;

		break;
	}

	case State::Paused: {
		if (input.IsKeyPressed(DirectX::Keyboard::Tab)) {
			ResumeGame();
			return;
		}

		if (input.IsKeyPressed(DirectX::Keyboard::Back)) {
			RequestSceneLoad(SceneIds::Title);
			return;
		}

		break;
	}

	case State::GameOver: {
		if (input.IsKeyPressed(DirectX::Keyboard::R)) {
			RestartGame();
			return;
		}

		if (input.IsKeyPressed(DirectX::Keyboard::Back)) {
			RequestSceneLoad(SceneIds::Title);
			return;
		}

		break;
	}
	}
}

void ShooterScene::OnRenderUI() {
	auto& spriteBatch = GetContext().spriteBatch;

	/*
	* continue displaying the game behind pause
	* and game-over messages
	*/
	m_shooterGame.Render(&spriteBatch);

	switch (m_state) {
	case State::Playing:
		DrawGameplayUI();
		break;

	case State::Paused:
		DrawPauseUI();
		break;

	case State::GameOver:
		DrawGameOverUI();
		break;
	}
}

bool ShooterScene::ShouldUpdateGameObjects() const noexcept {
	return m_state == State::Playing;
}

std::array<float, 4> ShooterScene::OnGetClearColor() const noexcept {
	return { 1.0f, 1.0f, 1.0f, 1.0f };
}

void ShooterScene::RestartGame() {
	m_shooterGame.Start();

	m_state = State::Playing;

	auto& input = InputManager::Get();

	input.SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
	input.Reset();
}

void ShooterScene::PauseGame() {
	if (m_state != State::Playing)
		return;

	m_state = State::Paused;

	InputManager::Get().SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
}

void ShooterScene::ResumeGame() {
	if (m_state != State::Paused)
		return;

	m_state = State::Playing;

	auto& input =
		InputManager::Get();

	input.SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
	input.Reset();
}

void ShooterScene::DrawGameplayUI() {
	using DirectX::SimpleMath::Vector2;

	auto& context = GetContext();

	auto& font = context.font;
	auto& spriteBatch = context.spriteBatch;

	const std::wstring scoreText = L"Score : " +
		std::to_wstring(m_shooterGame.GetScore());

	const std::wstring hpText = L"HP    : " +
		std::to_wstring(m_shooterGame.GetPlayerHP());

	font.DrawString(
		&spriteBatch,
		scoreText.c_str(),
		Vector2(20.0f, 20.0f),
		DirectX::Colors::DarkGreen
	);

	font.DrawString(
		&spriteBatch,
		hpText.c_str(),
		Vector2(20.0f, 60.0f),
		DirectX::Colors::DarkGreen
	);

	font.DrawString(
		&spriteBatch,
		L"WASD/Arrows : Move\n"
		L"SPACE       : Shoot\n"
		L"ESC         : Pause",
		Vector2(20.0f, 100.0f),
		DirectX::Colors::DarkMagenta
	);
}

void ShooterScene::DrawPauseUI() {
	using DirectX::SimpleMath::Vector2;

	auto& context =
		GetContext();

	auto& font =
		context.font;

	auto& spriteBatch =
		context.spriteBatch;

	const float windowWidth =
		static_cast<float>(
			context.deviceResources.GetWidth()
			);

	const float windowHeight =
		static_cast<float>(
			context.deviceResources.GetHeight()
			);

	constexpr const wchar_t* pauseText =
		L"PAUSED";

	constexpr const wchar_t* controlsText =
		L"TAB       : Resume\n"
		L"BACKSPACE : Return to Title";

	const Vector2 pauseSize =
		font.MeasureString(pauseText);

	const Vector2 controlsSize =
		font.MeasureString(controlsText);

	font.DrawString(
		&spriteBatch,
		pauseText,
		Vector2(
			windowWidth * 0.5f -
			pauseSize.x * 0.5f,
			windowHeight * 0.5f -
			70.0f
		),
		DirectX::Colors::DarkRed
	);

	font.DrawString(
		&spriteBatch,
		controlsText,
		Vector2(
			windowWidth * 0.5f -
			controlsSize.x * 0.5f,
			windowHeight * 0.5f
		),
		DirectX::Colors::DarkBlue
	);
}

void ShooterScene::DrawGameOverUI() {
	using DirectX::SimpleMath::Vector2;

	auto& context = GetContext();

	auto& font = context.font;
	auto& spriteBatch = context.spriteBatch;

	const float windowWidth = static_cast<float>(context.deviceResources.GetWidth());
	const float windowHeight = static_cast<float>(context.deviceResources.GetHeight());

	constexpr const wchar_t* gameOverText = L"GAME OVER";

	const std::wstring finalScoreText = L"Final Score: " +
		std::to_wstring(m_shooterGame.GetScore());

	constexpr const wchar_t* restartText = L"R         : Restart";

	constexpr const wchar_t* titleText = L"BACKSPACE : Title";

	const Vector2 gameOverSize = font.MeasureString(gameOverText);
	const Vector2 finalScoreSize = font.MeasureString(finalScoreText.c_str());
	const Vector2 restartSize = font.MeasureString(restartText);
	const Vector2 titleSize = font.MeasureString(titleText);

	font.DrawString(
		&spriteBatch,
		gameOverText,
		Vector2(
			windowWidth * 0.5f -
			gameOverSize.x * 0.5f,
			windowHeight * 0.5f -
			100.0f
		),
		DirectX::Colors::DarkOrange
	);

	font.DrawString(
		&spriteBatch,
		finalScoreText.c_str(),
		Vector2(
			windowWidth * 0.5f -
			finalScoreSize.x * 0.5f,
			windowHeight * 0.5f -
			40.0f
		),
		DirectX::Colors::DarkGreen
	);

	font.DrawString(
		&spriteBatch,
		restartText,
		Vector2(
			windowWidth * 0.5f -
			restartSize.x * 0.5f,
			windowHeight * 0.5f +
			20.0f
		),
		DirectX::Colors::DarkMagenta
	);

	font.DrawString(
		&spriteBatch,
		titleText,
		Vector2(
			windowWidth * 0.5f -
			titleSize.x * 0.5f,
			windowHeight * 0.5f +
			55.0f
		),
		DirectX::Colors::DarkMagenta
	);
}