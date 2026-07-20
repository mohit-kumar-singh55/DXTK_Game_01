#include "Game.h"

#include <DirectXColors.h>

#include <string>

void Game::Initialize(HWND window, int width, int height) {
	m_windowWidth = width;
	m_windowHeight = height;

	m_deviceResources.Initialize(window, width, height);

	// init singleton input manager
	InputManager::Get().Initialize(window);

	InitializeGameResources();

	m_audioManager.Initialize();

	m_previousTime = Clock::now();
}

void Game::Tick() {
	// calculate delta time
	auto currentTime = Clock::now();

	float deltaTime = std::chrono::duration<float>(
		currentTime - m_previousTime
	).count();

	m_previousTime = currentTime;

	// ! safety clamp
	// ! this prevents a huge jump if the program freezes for a moment
	deltaTime = (deltaTime < 0.1f) ? deltaTime : 0.1f;	// min

	Update(deltaTime);
	m_audioManager.Update();
	Render();
}

void Game::InitializeGameResources() {
	ID3D11Device* device = m_deviceResources.GetDevice();
	ID3D11DeviceContext* context = m_deviceResources.GetContext();

	// create DirectXTK objects
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(context);
	m_font = std::make_unique<DirectX::SpriteFont>(device, L"Assets/gamefont.spritefont");

	// ! 2d shooter game
	m_shooterGame.Initialize(device, m_windowWidth, m_windowHeight);

	// ! 3d tank game
	m_tankGame.Initialize(
		device,
		context,
		m_windowWidth,
		m_windowHeight
	);
}

void Game::Update(float deltaTime) {
	auto& input = InputManager::Get();
	input.Update();		// ! update input manage only once here, not in any other class

	// close window
	if (input.IsKeyPressed(DirectX::Keyboard::Escape))
		PostQuitMessage(0);

	// for 2D gae
	switch (m_gameState) {
	case GameState::Title: {
		if (input.IsKeyPressed(DirectX::Keyboard::D1))
			Start2DGame();
		if (input.IsKeyPressed(DirectX::Keyboard::D2))
			Start3DGame();
		break;
	}

	case GameState::Playing: {
		if (m_gameMode == GameMode::Shooter2D) {
			m_shooterGame.Update(
				deltaTime,
				m_audioManager
			);

			if (m_shooterGame.IsGameOver())
				m_gameState = GameState::GameOver;
		}

		if (m_gameMode == GameMode::Arena3D) {
			m_tankGame.Update(
				deltaTime,
				m_audioManager
			);

			if (m_tankGame.IsGameOver())
				m_gameState = GameState::GameOver;
		}
		break;
	}

	case GameState::GameOver: {
		if (input.IsKeyPressed(DirectX::Keyboard::R)) {
			if (m_gameMode == GameMode::Shooter2D)
				Start2DGame();
			if (m_gameMode == GameMode::Arena3D)
				Start3DGame();
		}
		if (input.IsKeyPressed(DirectX::Keyboard::Back))
			ReturnToTitle();
		break;
	}

	default:
		break;
	}
}

void Game::Render() {
	// bg color (just matching with the fog color, so fog could blend in
	const auto& clearColor3D = m_tankGame.GetClearColor();

	const float clearColor[4] = {
		clearColor3D.x,
		clearColor3D.y,
		clearColor3D.z,
		1.0f
	};

	// bind render targets, sets viewport and clears both buffers
	m_deviceResources.BeginFrame(clearColor);

	// ! render 3d
	if (m_gameMode == GameMode::Arena3D &&
		(m_gameState == GameState::Playing ||
			m_gameState == GameState::GameOver))
		m_tankGame.Render();

	// ! render 2d
	m_spriteBatch->Begin();

	if (m_gameMode == GameMode::Shooter2D &&
		(m_gameState == GameState::Playing ||
			m_gameState == GameState::GameOver))
		m_shooterGame.Render(m_spriteBatch.get());

	// draw ui
	DrawUI();

	m_spriteBatch->End();

	// swape chain present
	m_deviceResources.Present();
}

void Game::Start2DGame() {
	m_gameMode = GameMode::Shooter2D;
	m_gameState = GameState::Playing;

	m_shooterGame.Start();

	InputManager::Get().SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
}

void Game::Start3DGame() {
	m_gameMode = GameMode::Arena3D;
	m_gameState = GameState::Playing;

	m_tankGame.Start();

	// in relative mode, mouse only reports how much it moved this frame, not the actual screen position
	InputManager::Get().SetMouseMode(DirectX::Mouse::MODE_RELATIVE);
}

void Game::ReturnToTitle() {
	m_gameMode = GameMode::None;
	m_gameState = GameState::Title;

	m_tankGame.Clear();
	m_shooterGame.Clear();

	// reset mouse mode to absolute, so the mouse cursor can be used to click buttons
	InputManager::Get().SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
}

void Game::DrawUI() {
	if (!m_font) return;

	using DirectX::SimpleMath::Vector2;

	switch (m_gameState) {
	case GameState::Title: {
		// title
		m_font->DrawString(
			m_spriteBatch.get(),
			L"PROTOTYPE ALPHA",
			Vector2(
				m_windowWidth * 0.5f - 130.0f,
				m_windowHeight * 0.5f - 120.0f
			),
			DirectX::Colors::DarkBlue
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			L"Press 1: 2D Shooter",
			Vector2(
				m_windowWidth * 0.5f - 170.0f,
				m_windowHeight * 0.5f - 40.0f
			),
			DirectX::Colors::DarkMagenta
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			L"Press 2: 3D Arena",
			Vector2(
				m_windowWidth * 0.5f - 170.0f,
				m_windowHeight * 0.5f + 10.0f
			),
			DirectX::Colors::DarkMagenta
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			L"Esc : Quit",
			Vector2(
				m_windowWidth * 0.5f - 80.0f,
				m_windowHeight * 0.5f + 80.0f
			),
			DirectX::Colors::DarkRed
		);
		break;
	}

	case GameState::Playing: {
		if (m_gameMode == GameMode::Shooter2D) {
			const std::wstring scoreText = L"Score : " + std::to_wstring(m_shooterGame.GetScore());
			const std::wstring hpText = L"HP    : " + std::to_wstring(m_shooterGame.GetPlayerHP());

			m_font->DrawString(
				m_spriteBatch.get(),
				scoreText.c_str(),
				Vector2(20.0f, 20.0f),
				DirectX::Colors::DarkGreen
			);

			m_font->DrawString(
				m_spriteBatch.get(),
				hpText.c_str(),
				Vector2(20.0f, 60.0f),
				DirectX::Colors::DarkGreen
			);

			m_font->DrawString(
				m_spriteBatch.get(),
				L"WASD/Arrows : Move\nSPACE       : Shoot",
				Vector2(20.0f, 100.0f),
				DirectX::Colors::DarkMagenta
			);
		}

		if (m_gameMode == GameMode::Arena3D) {
			const std::wstring score3DText = L"3D Score : " + std::to_wstring(m_tankGame.GetScore());
			const std::wstring hp3DText = L"HP       : " + std::to_wstring(m_tankGame.GetPlayerHP());

			m_font->DrawString(
				m_spriteBatch.get(),
				score3DText.c_str(),
				Vector2(20.0f, 20.0f),
				DirectX::Colors::DarkGreen
			);

			m_font->DrawString(
				m_spriteBatch.get(),
				hp3DText.c_str(),
				Vector2(20.0f, 60.0f),
				DirectX::Colors::DarkGreen
			);

			m_font->DrawString(
				m_spriteBatch.get(),
				L"WASD/Arrows : Move\nMOUSE       : Look\nLEFT CLICK  : Shoot",
				Vector2(20.0f, 100.0f),
				DirectX::Colors::DarkMagenta
			);

			if (m_tankGame.IsTankDestroyed()) {
				m_font->DrawString(
					m_spriteBatch.get(),
					L"TANK DESTROYED...",
					Vector2(300.0f, 250.0f),
					DirectX::Colors::Yellow
				);
			}
		}
		break;
	}

	case GameState::GameOver: {
		std::wstring finalScoreText;

		if (m_gameMode == GameMode::Shooter2D)
			finalScoreText = L"Final Score:" + std::to_wstring(m_shooterGame.GetScore());
		else if (m_gameMode == GameMode::Arena3D)
			finalScoreText = L"Final Score:" + std::to_wstring(m_tankGame.GetScore());

		m_font->DrawString(
			m_spriteBatch.get(),
			L"GAME OVER",
			Vector2(
				m_windowWidth * 0.5f - 80.0f,
				m_windowHeight * 0.5f - 100.0f
			),
			DirectX::Colors::DarkOrange
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			finalScoreText.c_str(),
			Vector2(
				m_windowWidth * 0.5f - 130.0f,
				m_windowHeight * 0.5f - 40.0f
			),
			DirectX::Colors::DarkGreen
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			L"R         : Restart",
			Vector2(
				m_windowWidth * 0.5f - 150.0f,
				m_windowHeight * 0.5f + 20.0f
			),
			DirectX::Colors::DarkMagenta
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			L"Backspace : Title",
			Vector2(
				m_windowWidth * 0.5f - 150.0f,
				m_windowHeight * 0.5f + 50.0f
			),
			DirectX::Colors::DarkMagenta
		);
		break;
	}
	}
}