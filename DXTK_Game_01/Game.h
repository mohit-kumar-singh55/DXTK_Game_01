#pragma once

#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "AudioManager.h"

#include <Games/TankGame3D.h>

#define WIN32_LEAN_AND_MEAN

// window related
#include <windows.h>
#include <wrl/client.h>

#include <d3d11.h>

#include <Graphics/DeviceResources.h>

// DirectXTK
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <Keyboard.h>
#include <Mouse.h>

// game related
#include <chrono>
#include <vector>
#include <random>

class Game {
public:
	Game() = default;

	void Initialize(HWND window, int width, int height);

	void Tick();

private:
	void InitializeGameResources();

	void Update(float deltaTime);
	void Render();

	void Start2DGame();
	void Start3DGame();
	void ReturnToTitle();

	void Update2D(
		float deltaTime,
		const DirectX::Keyboard::State& keyboardState
	);

	void Render2D();

	void SpawnEnemy();

	void DrawUI();

	void CreateBulletTexture();
	void CreateEnemyTexture();

private:
	int m_windowWidth = 1280;
	int m_windowHeight = 720;

	// device related
	DeviceResources m_deviceResources;

	// time related
	using Clock = std::chrono::steady_clock;
	Clock::time_point m_previousTime;						// used for delta time

	// DirectXTK objects
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_font;	// ui text renderer
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;
	DirectX::Mouse::ButtonStateTracker m_mouseTracker;

	static constexpr float MouseSensitivity = 0.0035f;

	// 3D tank game
	TankGame3D m_tankGame;

	std::unique_ptr<Player> m_player;

	std::vector<Bullet> m_bullets;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bulletTexture;

	std::vector<Enemy> m_enemies;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_enemyTexture;
	float m_enemySpawnTimer = 0.0f;
	static constexpr float ENEMY_SPAWN_INTERVAL = 1.0f;
	std::mt19937 m_randomEngine{ std::random_device{}() };

	int m_score = 0;

	static constexpr int PLAYER_MAX_HP = 3;
	int m_playerHp = PLAYER_MAX_HP;

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