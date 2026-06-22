#pragma once

#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"

#define WIN32_LEAN_AND_MEAN

// window related
#include <windows.h>
#include <wrl/client.h>

#include <d3d11.h>
#include <dxgi.h>

// DirectXTK
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <DirectXColors.h>

// game related
#include <chrono>
#include <vector>
#include <cstdint>
#include <random>

class Game {
public:
	Game() = default;

	void Initialize(HWND window, int width, int height);

	void Tick();

private:
	void InitializeDirect3D();

	void Update(float deltaTime);
	void Render();

	void StartGame();

	void SpawnEnemy();

	void DrawUI();

	void CreateBulletTexture();
	void CreateEnemyTexture();

	static void ThrowIfFailed(HRESULT result);

private:
	HWND m_window = nullptr;								// window handle, needs a window before we can render it

	int m_windowWidth = 1280;
	int m_windowHeight = 720;

	Microsoft::WRL::ComPtr<ID3D11Device> m_device;							// device creates GPU resources			(context sends commands to the GPU)
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;					// context sends commands to the GPU		(Context = GPU command sender)
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;						// swap chain owns the screen buffers	(Swap chain = screen buffer manager)
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;		// This is the image we are drawing into	(In Unity terms, it is similar t

	// game related
	using Clock = std::chrono::steady_clock;
	Clock::time_point m_previousTime;						// used for delta time

	// DirectXTK objects
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::Keyboard> m_keyboard;

	std::unique_ptr<Player> m_player;

	std::vector<Bullet> m_bullets;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bulletTexture;
	DirectX::Keyboard::KeyboardStateTracker m_keyboardTracker;

	std::vector<Enemy> m_enemies;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_enemyTexture;
	float m_enemySpawnTimer = 0.0f;
	static constexpr float ENEMY_SPAWN_INTERVAL = 1.0f;
	std::mt19937 m_randomEngine{ std::random_device{}() };

	std::unique_ptr<DirectX::SpriteFont> m_font;	// ui text renderer

	int m_score = 0;

	static constexpr int PLAYER_MAX_HP = 3;
	int m_playerHp = PLAYER_MAX_HP;

	enum class GameState {
		Title,
		Playing,
		GameOver
	};

	GameState m_gameState = GameState::Title;
};