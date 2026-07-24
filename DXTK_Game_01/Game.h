#pragma once

#define WIN32_LEAN_AND_MEAN

// window related
#include <windows.h>

#include <SNX/Graphics/DeviceResources.h>

// DirectXTK
#include <SpriteBatch.h>
#include <SpriteFont.h>

// game related
#include <SNX/Core/Object/GameObjectManager.h>
#include <SNX/Audio/AudioManager.h>
#include <Games/Tank3D/TankGame3D.h>
#include <Games/Shooter2D/ShooterGame2D.h>

class Game final {
public:
	Game() = default;
	~Game() = default;

	// disallowing to copy or move
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	Game(Game&&) = delete;
	Game& operator=(Game&&) = delete;

	void Initialize(HWND window, int width, int height);

	void Tick();

private:
	void InitializeGameResources();

	void Update();
	void FixedUpdate();
	void Render();

	void Start2DGame();
	void Start3DGame();
	void ReturnToTitle();

	void DrawUI();

private:
	int m_windowWidth = 1280;
	int m_windowHeight = 720;

	// device related
	DeviceResources m_deviceResources;

	// DirectXTK objects
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_font;	// ui text renderer

	static constexpr float MouseSensitivity = 0.0035f;

	GameObjectManager m_gameObjects;

	// 3D tank game
	TankGame3D m_tankGame;
	// 2d shooter game
	ShooterGame2D m_shooterGame;

	enum class GameState {
		Title,
		Playing,
		GameOver
	};

	enum class GameMode {
		None,
		Shooter2D,
		Arena3D
	};

	GameState m_gameState = GameState::Title;
	GameMode m_gameMode = GameMode::None;

	AudioManager m_audioManager;
};