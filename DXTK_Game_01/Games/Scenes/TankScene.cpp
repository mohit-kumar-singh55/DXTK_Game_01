#include "TankScene.h"

#include "SceneIds.h"

#include <SNX/Core/Time.h>
#include <SNX/Graphics/DeviceResources.h>
#include <SNX/Graphics/RenderContext.h>
#include <SNX/Input/InputManager.h>

#include <DirectXColors.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

#include <string>

TankScene::TankScene(SceneManager& sceneManager, SceneContext& context) noexcept :
	Scene(sceneManager, context) {}

void TankScene::OnLoad() {
	auto& context = GetContext();

	auto& deviceResources = context.deviceResources;

	m_tankGame.Initialize(
		deviceResources.GetDevice(),
		deviceResources.GetContext(),
		deviceResources.GetWidth(),
		deviceResources.GetHeight()
	);

	m_tankGame.Start(GetGameObjects());

	m_state = State::Playing;

	auto& input = InputManager::Get();

	input.SetMouseMode(DirectX::Mouse::MODE_RELATIVE);
	input.Reset();
}

void TankScene::OnUnload() {
	/*
	* Reset TankGame3D's non-owning GameObject pointers
	* before Scene clears the actual GameObjects
	*/
	m_tankGame.Clear();
}

void TankScene::OnUpdate() {
	auto& input = InputManager::Get();

	switch (m_state) {
	case State::Playing: {
		if (input.IsKeyPressed(DirectX::Keyboard::Escape)) {
			PauseGame();
			return;
		}

		m_tankGame.Update(Time::DeltaTime(), GetContext().audioManager);

		if (m_tankGame.IsGameOver())
			EnterGameOver();

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
			RequestSceneLoad(
				SceneIds::Title
			);

			return;
		}

		break;
	}
	}
}

void TankScene::PauseGame() {
	if (m_state != State::Playing)
		return;

	m_state = State::Paused;

	InputManager::Get().SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
}

void TankScene::ResumeGame() {
	if (m_state != State::Paused)
		return;

	m_state = State::Playing;

	auto& input = InputManager::Get();

	input.SetMouseMode(DirectX::Mouse::MODE_RELATIVE);
	// remove mouse movement accumulated while paused
	input.Reset();
}

void TankScene::EnterGameOver() {
	if (m_state == State::GameOver)
		return;

	m_state = State::GameOver;

	auto& input = InputManager::Get();

	input.SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
	input.Reset();
}

void TankScene::RestartGame() {
	/*
	* reset all TankGame3D runtime data and its
	* non-owning references first
	*/
	m_tankGame.Clear();

	// destroy arena and tank component GameObjects
	GetGameObjects().Clear();

	// recreate a fresh arena and tank hierarchy
	m_tankGame.Start(GetGameObjects());

	m_state = State::Playing;

	auto& input = InputManager::Get();

	input.SetMouseMode(DirectX::Mouse::MODE_RELATIVE);
	input.Reset();
}

void TankScene::OnRenderWorld() {
	auto& deviceResources = GetContext().deviceResources;

	const Camera3D& camera = m_tankGame.GetCamera();

	RenderContext renderContext;

	renderContext.device = deviceResources.GetDevice();
	renderContext.deviceContext = deviceResources.GetContext();
	renderContext.view = camera.GetView();
	renderContext.projection = camera.GetProjection();
	renderContext.cameraPosition = camera.GetPosition();
	renderContext.viewportWidth = deviceResources.GetWidth();
	renderContext.viewportHeight = deviceResources.GetHeight();

	renderContext.fixedInterpolationAlpha = Time::FixedInterpolationAlpha();

	// component-managed arena and tank meshes
	GetGameObjects().Render(renderContext);

	// existing enemies, bullets, effects and fake shadow
	m_tankGame.Render();
}

bool TankScene::ShouldUpdateGameObjects() const noexcept {
	return m_state == State::Playing;
}

std::array<float, 4>
TankScene::OnGetClearColor()const noexcept {
	const auto& color = m_tankGame.GetClearColor();
	return { color.x, color.y, color.z,	1.0f };
}

void TankScene::OnRenderUI() {
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

void TankScene::DrawGameplayUI() {
	using DirectX::SimpleMath::Vector2;

	auto& context = GetContext();

	auto& font = context.font;
	auto& spriteBatch = context.spriteBatch;

	const std::wstring scoreText = L"3D Score : " +
		std::to_wstring(m_tankGame.GetScore());

	const std::wstring hpText = L"HP       : " +
		std::to_wstring(m_tankGame.GetPlayerHP());

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
		L"WASD  : Move\n"
		L"MOUSE : Aim\n"
		L"LMB   : Shoot\n"
		L"ESC   : Pause",
		Vector2(20.0f, 100.0f),
		DirectX::Colors::DarkMagenta
	);

	if (m_tankGame.IsTankDestroyed()) {
		constexpr const wchar_t* destroyedText = L"TANK DESTROYED...";

		const Vector2 size = font.MeasureString(destroyedText);

		const float windowWidth = static_cast<float>(context.deviceResources.GetWidth());

		font.DrawString(
			&spriteBatch,
			destroyedText,
			Vector2(
				windowWidth * 0.5f -
				size.x * 0.5f,
				250.0f
			),
			DirectX::Colors::Yellow
		);
	}
}

void TankScene::DrawPauseUI() {
	using DirectX::SimpleMath::Vector2;

	auto& context = GetContext();

	auto& font = context.font;
	auto& spriteBatch = context.spriteBatch;

	const float width = static_cast<float>(context.deviceResources.GetWidth());
	const float height = static_cast<float>(context.deviceResources.GetHeight());

	constexpr const wchar_t* pauseText = L"PAUSED";

	constexpr const wchar_t* controlsText =
		L"TAB       : Resume\n"
		L"BACKSPACE : Return to Title";

	const Vector2 pauseSize = font.MeasureString(pauseText);
	const Vector2 controlsSize = font.MeasureString(controlsText);

	font.DrawString(
		&spriteBatch,
		pauseText,
		Vector2(
			width * 0.5f -
			pauseSize.x * 0.5f,
			height * 0.5f -
			70.0f
		),
		DirectX::Colors::DarkRed
	);

	font.DrawString(
		&spriteBatch,
		controlsText,
		Vector2(
			width * 0.5f -
			controlsSize.x * 0.5f,
			height * 0.5f
		),
		DirectX::Colors::DarkBlue
	);
}

void TankScene::DrawGameOverUI()
{
	using DirectX::SimpleMath::Vector2;

	auto& context =
		GetContext();

	auto& font =
		context.font;

	auto& spriteBatch =
		context.spriteBatch;

	const float width =
		static_cast<float>(
			context.deviceResources.GetWidth()
			);

	const float height =
		static_cast<float>(
			context.deviceResources.GetHeight()
			);

	constexpr const wchar_t* gameOverText =
		L"GAME OVER";

	const std::wstring finalScoreText =
		L"Final Score: " +
		std::to_wstring(
			m_tankGame.GetScore()
		);

	constexpr const wchar_t* restartText =
		L"R         : Restart";

	constexpr const wchar_t* titleText =
		L"BACKSPACE : Title";

	const Vector2 gameOverSize =
		font.MeasureString(gameOverText);

	const Vector2 scoreSize =
		font.MeasureString(
			finalScoreText.c_str()
		);

	const Vector2 restartSize =
		font.MeasureString(restartText);

	const Vector2 titleSize =
		font.MeasureString(titleText);

	font.DrawString(
		&spriteBatch,
		gameOverText,
		Vector2(
			width * 0.5f -
			gameOverSize.x * 0.5f,
			height * 0.5f -
			100.0f
		),
		DirectX::Colors::DarkOrange
	);

	font.DrawString(
		&spriteBatch,
		finalScoreText.c_str(),
		Vector2(
			width * 0.5f -
			scoreSize.x * 0.5f,
			height * 0.5f -
			40.0f
		),
		DirectX::Colors::DarkGreen
	);

	font.DrawString(
		&spriteBatch,
		restartText,
		Vector2(
			width * 0.5f -
			restartSize.x * 0.5f,
			height * 0.5f +
			20.0f
		),
		DirectX::Colors::DarkMagenta
	);

	font.DrawString(
		&spriteBatch,
		titleText,
		Vector2(
			width * 0.5f -
			titleSize.x * 0.5f,
			height * 0.5f +
			55.0f
		),
		DirectX::Colors::DarkMagenta
	);
}