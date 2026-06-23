#include "Game.h"

#include "Collision.h"

#include <DirectXColors.h>

#include <algorithm>
#include <stdexcept>
#include <string>

#include "TextureFactory.h"

void Game::Initialize(HWND window, int width, int height) {
	m_window = window;
	m_windowWidth = width;
	m_windowHeight = height;

	InitializeDirect3D();

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
	Render();
}

void Game::InitializeDirect3D() {
	// create swap chain settings instance
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	// swap chain buffer settings
	swapChainDesc.BufferDesc.Width = m_windowWidth;
	swapChainDesc.BufferDesc.Height = m_windowHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;			// ! FPS
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	// swap chain sample settings
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// swap chain settings
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_window;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// define feature levels
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
	};

	// define created feature level
	D3D_FEATURE_LEVEL createdFeatureLevel{};

	// create device and swap chain
	ThrowIfFailed(
		D3D11CreateDeviceAndSwapChain(
			nullptr,                    // Adapter (mean GPU) if nullptr -> DX will choose itself
			D3D_DRIVER_TYPE_HARDWARE,   // Use GPU hardware
			nullptr,                    // Software rasterizer (Rasterizer is the stage in the graphics pipeline that converts triangles into pixels (fragments))
			0,                          // Flags
			featureLevels,
			1,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&m_swapChain,
			&m_device,
			&createdFeatureLevel,
			&m_context
		)
	);

	// Get back buffer from swap chain.
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

	ThrowIfFailed(
		m_swapChain->GetBuffer(
			0,
			IID_PPV_ARGS(&backBuffer)
		)
	);

	// Create render target view.
	ThrowIfFailed(
		m_device->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_renderTargetView
		)
	);

	// TODO:  from here create DirectXTK objects.
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_context.Get());
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_font = std::make_unique<DirectX::SpriteFont>(m_device.Get(), L"Assets/gamefont.spritefont");

	// create player and its texture
	m_player = std::make_unique<Player>();
	m_player->Initialize(m_device.Get());

	// create the shared bullet texture
	CreateBulletTexture();
	// create the shared enemy texture
	CreateEnemyTexture();
}

void Game::Update(float deltaTime) {
	// use keyboardState for continuous actions (will be true for all frame if the key is held)
	// use keyboardTracker for per frame actions (will be true only for the frame when the key is pressed)
	const auto keyboardState = m_keyboard->GetState();

	// compare the current keyboard state with the previous frame
	m_keyboardTracker.Update(keyboardState);

	// close window
	if (keyboardState.Escape)
		PostQuitMessage(0);

	switch (m_gameState) {
	case GameState::Title: {
		if (m_keyboardTracker.pressed.Enter)
			StartGame();
		break;
	}

						 // update things
	case GameState::Playing: {
		// update objects
		m_player->Update(keyboardState, deltaTime, m_windowWidth, m_windowHeight);

		// spawn bullet only once when space changes from released to pressed
		if (m_keyboardTracker.pressed.Space)
			// emplace_back constructs the object and push at the back
			m_bullets.emplace_back(m_player->GetBulletSpawnPosition());

		// update bullets
		for (Bullet& bullet : m_bullets)
			bullet.Update(deltaTime);

		// spawn enemies
		m_enemySpawnTimer += deltaTime;
		if (m_enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
			m_enemySpawnTimer -= ENEMY_SPAWN_INTERVAL;
			SpawnEnemy();
		}

		// update enemies
		for (Enemy& enemy : m_enemies)
			enemy.Update(deltaTime);

		// enemy reaching bottom damages the player
		for (Enemy& enemy : m_enemies) {
			if (!enemy.IsActive())
				continue;

			if (enemy.IsOutsideScreen(m_windowHeight)) {
				enemy.Destroy();
				--m_playerHp;
				if (m_playerHp <= 0) {
					m_playerHp = 0;
					m_gameState = GameState::GameOver;
				}
			}
		}

		// bullet vs enemy collision
		for (Bullet& bullet : m_bullets) {
			if (!bullet.IsActive())
				continue;

			for (Enemy& enemy : m_enemies) {
				if (!enemy.IsActive())
					continue;

				if (Intersects(bullet.GetBounds(), enemy.GetBounds())) {
					OutputDebugStringA("COLLIDED!!!\n");
					bullet.Destroy();
					enemy.Destroy();
					m_score += 100;
					break;
				}
			}
		}

		// or instead use erase_if for shorter version
		m_bullets.erase(
			// rearranges the vector so unwanted objects are moved to the end, then returns the beginning of that unwanted range
			// after that, erase truly removes them
			std::remove_if(
				m_bullets.begin(),
				m_bullets.end(),
				[](const Bullet& bullet) {
					return
						!bullet.IsActive() ||
						bullet.IsOutsideScreen();
				}
			),
			m_bullets.end()
		);

		// erase enemies if not active
		m_enemies.erase(
			// rearranges the vector so unwanted objects are moved to the end, then returns the beginning of that unwanted range
			// after that, erase truly removes them
			std::remove_if(
				m_enemies.begin(),
				m_enemies.end(),
				[](const Enemy& enemy) {
					return
						!enemy.IsActive();
				}
			),
			m_enemies.end()
		);
		break;
	}

	case GameState::GameOver: {
		if (m_keyboardTracker.pressed.R)
			StartGame();
		break;
	}

	default:
		break;
	}
}

void Game::Render() {
	const float clearColor[4] = {
		//0.1f, 0.15f, 0.25f, 1.0f
		1.0f, 0.15f, 0.25f, 1.0f
	};

	m_context->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		nullptr
	);

	// set the viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(m_windowWidth);
	viewport.Height = static_cast<float>(m_windowHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_context->RSSetViewports(1, &viewport);

	m_context->ClearRenderTargetView(
		m_renderTargetView.Get(),
		clearColor
	);

	// DirectXTK SpriteBatch
	m_spriteBatch->Begin();

	if (m_gameState == GameState::Playing || m_gameState == GameState::GameOver) {
		// draw enemies
		for (const Enemy& enemy : m_enemies)
			enemy.Draw(m_spriteBatch.get(), m_enemyTexture.Get());

		// drawing bullets before player, so that the player overlap the bullets
		for (const Bullet& bullet : m_bullets)
			bullet.Draw(m_spriteBatch.get(), m_bulletTexture.Get());

		// draw player
		m_player->Draw(m_spriteBatch.get());
	}

	// draw ui
	DrawUI();

	m_spriteBatch->End();

	m_swapChain->Present(1, 0);
}

void Game::StartGame() {
	m_score = 0;
	m_playerHp = PLAYER_MAX_HP;

	m_bullets.clear();
	m_enemies.clear();

	m_enemySpawnTimer = 0.0f;

	if (m_player)
		m_player->Reset();

	m_gameState = GameState::Playing;
}

void Game::SpawnEnemy() {
	// Generate random float values between 0.0f and WINDOW_WIDTH - Enemy::TextureWidth
	std::uniform_real_distribution<float> distribution(
		0.0f,
		static_cast<float>(m_windowWidth - Enemy::TextureWidth)
	);

	const float x = distribution(m_randomEngine);

	DirectX::SimpleMath::Vector2 spawnPos{
		x,
		-static_cast<float>(Enemy::TextureHeight)
	};

	m_enemies.emplace_back(spawnPos);
}

void Game::DrawUI() {
	if (!m_font) return;

	using DirectX::SimpleMath::Vector2;

	switch (m_gameState) {
	case GameState::Title: {
		// title
		m_font->DrawString(
			m_spriteBatch.get(),
			L"DXTK SHOOTER",
			Vector2(
				m_windowWidth * 0.5f - 80.0f,
				m_windowHeight * 0.5f - 150.0f
			),
			DirectX::Colors::White
		);

		// instructon
		m_font->DrawString(
			m_spriteBatch.get(),
			L"Press ENTER to Start",
			Vector2(
				m_windowWidth * 0.5f - 170.0f,
				m_windowHeight * 0.5f - 20.0f
			),
			DirectX::Colors::Yellow
		);

		// controls
		m_font->DrawString(
			m_spriteBatch.get(),
			L"WASD / Arrows: Move\nSpace: Shoot\nEsc: Quit",
			Vector2(
				m_windowWidth * 0.5f - 400.0f,
				m_windowHeight * 0.5f + 150.0f
			),
			DirectX::Colors::White
		);
		break;
	}

	case GameState::Playing: {
		const std::wstring scoreText = L"Score: " + std::to_wstring(m_score);
		const std::wstring hpText = L"HP: " + std::to_wstring(m_playerHp);

		// draw strings
		m_font->DrawString(
			m_spriteBatch.get(),
			scoreText.c_str(),
			DirectX::SimpleMath::Vector2(20.0f, 20.0f),
			DirectX::Colors::White
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			hpText.c_str(),
			DirectX::SimpleMath::Vector2(20.0f, 60.0f),
			DirectX::Colors::White
		);
		break;
	}

	case GameState::GameOver: {
		const std::wstring scoreText = L"Final Score:" + std::to_wstring(m_score);

		m_font->DrawString(
			m_spriteBatch.get(),
			L"GAME OVER",
			Vector2(
				m_windowWidth * 0.5f - 80.0f,
				m_windowHeight * 0.5f - 100.0f
			),
			DirectX::Colors::Yellow
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			scoreText.c_str(),
			Vector2(
				m_windowWidth * 0.5f - 130.0f,
				m_windowHeight * 0.5f - 40.0f
			),
			DirectX::Colors::White
		);

		m_font->DrawString(
			m_spriteBatch.get(),
			L"Press R to Restart",
			Vector2(
				m_windowWidth * 0.5f - 150.0f,
				m_windowHeight * 0.5f + 20.0f
			),
			DirectX::Colors::White
		);
		break;
	}
	}
}

void Game::CreateBulletTexture() {
	// actual image texture
	try {
		m_bulletTexture = TextureFactory::LoadTextureFromFile(
			m_device.Get(),
			L"Assets/Textures/bullet.png"
		);
	}
	// fallback debug texture
	catch (...) {
		m_bulletTexture = TextureFactory::CreateRectangleTexture(
			m_device.Get(),
			Bullet::TextureWidth,
			Bullet::TextureHeight,
			TextureFactory::ColorRGBA{ 255, 210, 50, 255 },
			TextureFactory::ColorRGBA{ 255, 255, 255, 255 },
			1
		);
	}
}

void Game::CreateEnemyTexture() {
	// actual image texture
	try {
		m_enemyTexture = TextureFactory::LoadTextureFromFile(
			m_device.Get(),
			L"Assets/Textures/enemy.png"
		);
	}
	// fallback debug texture
	catch (...) {
		m_enemyTexture = TextureFactory::CreateRectangleTexture(
			m_device.Get(),
			Enemy::TextureWidth,
			Enemy::TextureHeight,
			TextureFactory::ColorRGBA{ 255, 80, 100, 255 },
			TextureFactory::ColorRGBA{ 255, 255, 255, 255 },
			4
		);
	}
}

void Game::ThrowIfFailed(HRESULT result) {
	if (FAILED(result)) {
		throw std::runtime_error("HRESULT failed.");
	}
}