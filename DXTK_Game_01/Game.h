#pragma once

#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "AudioManager.h"

#include "Player3D.h"
#include "Enemy3D.h"
#include "Bullet3D.h"
#include "GroundObject.h"
#include "WallObject.h"
#include "Camera3D.h"
#include "TankVisual.h"

#include "BlobShadow.h"
#include "MuzzleFlash3D.h"
#include "Explosion3D.h"
#include "DamageFlash3D.h"

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
#include <Mouse.h>
#include <GeometricPrimitive.h>
#include <Effects.h>
#include <CommonStates.h>

// game related
#include <chrono>
#include <vector>
#include <random>
#include <array>

class Game {
public:
	Game() = default;

	void Initialize(HWND window, int width, int height);

	void Tick();

private:
	void InitializeDirect3D();
	void CreateDepthBuffer();		// for 3d purpose
	void Initialize3D();			// for 3d purpose
	void InitializeBasicEffect();	// for dxtk's build-in matarial+shader

	void Update(float deltaTime);
	void Render();

	void Start2DGame();
	void Start3DGame();
	void ReturnToTitle();

	void Update2D(
		float deltaTime,
		const DirectX::Keyboard::State& keyboardState
	);
	void Update3D(
		float deltaTime,
		const DirectX::Keyboard::State& keyboardState,
		const DirectX::Mouse::State& mouseState
	);

	void Render2D();
	void Render3D();

	void SpawnEnemy();
	void SpawnEnemy3D();

	void DrawUI();

	void CreateBulletTexture();
	void CreateEnemyTexture();

	static void ThrowIfFailed(HRESULT result);

	void DestroyTank3D();

private:
	HWND m_window = nullptr;								// window handle, needs a window before we can render it

	int m_windowWidth = 1280;
	int m_windowHeight = 720;

	Microsoft::WRL::ComPtr<ID3D11Device> m_device;							// device creates GPU resources			(context sends commands to the GPU)
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;					// context sends commands to the GPU		(Context = GPU command sender)
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;						// swap chain owns the screen buffers	(Swap chain = screen buffer manager)
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;		// This is the image we are drawing into	(In Unity terms, it is similar to )
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;		// In 3d, depth buffer decides which object is in front and which object is behind

	// game related
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

	// for 3d purpose
	Camera3D m_cam;

	// dxtk's build-in material+shader
	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_basicEffectInputLayout;

	// fog purpose
	//DirectX::SimpleMath::Vector3 m_fogColor = DirectX::SimpleMath::Vector3(0.05f, 0.06f, 0.08f);
	DirectX::SimpleMath::Vector3 m_fogColor = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);
	float m_fogStart = 6.0f;
	float m_fogEnd = 18.0f;

	Player3D m_player3D;
	GroundObject m_ground;
	std::array<WallObject, 4> m_walls;
	std::vector<Enemy3D> m_enemies3D;
	std::vector<Bullet3D> m_bullets3D;
	std::vector<Explosion3D> m_explosions3D;

	std::unique_ptr<DirectX::GeometricPrimitive> m_bullet3DPrimitive;
	std::unique_ptr<DirectX::GeometricPrimitive> m_explosion3DPrimitive;

	float m_enemy3DSpawnTimer = 0.0f;
	static constexpr float Enemy3DSpawnInterval = 2.0f;
	static constexpr float Enemy3DSpawnMinDistanceFromPlayer = 6.0f;
	static constexpr float Enemy3DSpawnEdge = 8.5f;

	static constexpr int Player3DMaxHp = 3;
	int m_player3DHp = Player3DMaxHp;

	int m_score3D = 0;
	
	bool m_isTankDestroyed = false;
	float m_tankDeathTimer = 0.0f;

	static constexpr float TankDeathGameOverDelay = 1.4f;

	std::unique_ptr<DirectX::CommonStates> m_commonStates;
	std::unique_ptr<DirectX::EffectFactory> m_modelEffectfactory;

	TankVisual m_tankVisual;
	BlobShadow m_playerShadow;
	MuzzleFlash3D m_muzzleFlash;
	DamageFlash3D m_damageFlash;
	// upto here

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