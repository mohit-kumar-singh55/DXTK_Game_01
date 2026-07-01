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

	// Get back buffer from swap chain
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;

	ThrowIfFailed(
		m_swapChain->GetBuffer(
			0,
			IID_PPV_ARGS(&backBuffer)
		)
	);

	// Create render target view
	ThrowIfFailed(
		m_device->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_renderTargetView
		)
	);

	// create depth buffer for 3D purpose
	CreateDepthBuffer();

	// TODO:  from here create DirectXTK objects.
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_context.Get());
	m_keyboard = std::make_unique<DirectX::Keyboard>();
	m_mouse = std::make_unique<DirectX::Mouse>();
	m_font = std::make_unique<DirectX::SpriteFont>(m_device.Get(), L"Assets/gamefont.spritefont");

	m_mouse->SetWindow(m_window);

	// create player and its texture
	m_player = std::make_unique<Player>();
	m_player->Initialize(m_device.Get());

	// create the shared bullet texture
	CreateBulletTexture();
	// create the shared enemy texture
	CreateEnemyTexture();

	// ! creating 3D world, camera
	Initialize3D();
}

/*
* Render target = color image (what color is this pixel?)
* Depth buffer  = distance image (how far away is this pixel?)
*/
void Game::CreateDepthBuffer() {
	D3D11_TEXTURE2D_DESC depthTextureDesc = {};

	depthTextureDesc.Width = m_windowWidth;
	depthTextureDesc.Height = m_windowHeight;
	depthTextureDesc.MipLevels = 1;
	depthTextureDesc.ArraySize = 1;
	depthTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthTextureDesc.SampleDesc.Count = 1;
	depthTextureDesc.SampleDesc.Quality = 0;
	depthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;

	ThrowIfFailed(
		m_device->CreateTexture2D(
			&depthTextureDesc,
			nullptr,
			&depthTexture
		)
	);

	ThrowIfFailed(
		m_device->CreateDepthStencilView(
			depthTexture.Get(),
			nullptr,
			&m_depthStencilView
		)
	);
}

void Game::Initialize3D() {
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	// create a cube
	m_player3D.Initialize(m_context.Get());
	m_player3D.SetPosition(Vector3::Zero);
	m_player3D.SetMovementBounds(-8.5f, 8.5f, -8.5f, 8.5f);

	// initialize bullet
	m_bullet3DPrimitive = DirectX::GeometricPrimitive::CreateSphere(
		m_context.Get(),
		1.0f
	);

	// create ground
	m_ground.Initialize(m_context.Get());

	// walls
	for (WallObject& wall : m_walls)
		wall.Initialize(m_context.Get());

	// initialize basic effect
	InitializeBasicEffect();

	// set walls pos
	// back wall
	m_walls[0].SetTransform(
		Vector3(0.0f, 0.0f, -10.0f),
		Vector3(20.0f, 2.0f, 0.3f)
	);
	// front wall
	m_walls[1].SetTransform(
		Vector3(0.0f, 0.0f, 10.0f),
		Vector3(20.0f, 2.0f, 0.3f)
	);
	// left wall
	m_walls[2].SetTransform(
		Vector3(-10.0f, 0.0f, 0.0f),
		Vector3(0.3f, 2.0f, 20.0f)
	);
	// right wall
	m_walls[3].SetTransform(
		Vector3(10.0f, 0.0f, 0.0f),
		Vector3(0.3f, 2.0f, 20.0f)
	);

	const float aspectRatio =
		static_cast<float>(m_windowWidth) /
		static_cast<float>(m_windowHeight);

	// 3D perspective lens (camera lens)
	m_cam.SetPerspective(60.0f, aspectRatio, 0.1f, 100.0f);

	// camera offset from the player
	m_cam.SetFollowOffset(Vector3(0.0f, 6.0f, 8.0f));

	// update camera
	m_cam.FollowBehind(
		m_player3D.GetPosition(),
		m_player3D.GetForwardDirection()
	);
}

void Game::InitializeBasicEffect() {
	m_basicEffect = std::make_unique<DirectX::BasicEffect>(m_device.Get());

	// lightning
	m_basicEffect->SetLightingEnabled(true);
	m_basicEffect->SetPerPixelLighting(true);
	m_basicEffect->EnableDefaultLighting();

	m_basicEffect->SetAmbientLightColor(DirectX::SimpleMath::Vector3(0.25f, 0.25f, 0.25f));
	m_basicEffect->SetSpecularColor(DirectX::SimpleMath::Vector3(0.3f, 0.3f, 0.3f));

	m_basicEffect->SetSpecularPower(16.0f);

	// fog
	m_basicEffect->SetFogEnabled(true);
	m_basicEffect->SetFogStart(m_fogStart);
	m_basicEffect->SetFogEnd(m_fogEnd);
	m_basicEffect->SetFogColor(m_fogColor);

	// ! because all GeometricPrimitive shapes use the same built-in "vertex data structure"
	// ! so one input layout is enough for all of our current primitve objects
	m_ground.CreateInputLayout(
		m_basicEffect.get(),
		m_basicEffectInputLayout.GetAddressOf()
	);
}

void Game::Update(float deltaTime) {
	// use keyboardState for continuous actions (will be true for all frame if the key is held)
	// use keyboardTracker for per frame actions (will be true only for the frame when the key is pressed)
	const auto keyboardState = m_keyboard->GetState();
	const auto mouseState = m_mouse->GetState();

	// compare the current keyboard state with the previous frame
	m_keyboardTracker.Update(keyboardState);
	m_mouseTracker.Update(mouseState);

	// close window
	if (keyboardState.Escape)
		PostQuitMessage(0);

	// for 2D gae
	switch (m_gameState) {
	case GameState::Title: {
		if (m_keyboardTracker.pressed.D1)
			Start2DGame();
		if (m_keyboardTracker.pressed.D2)
			Start3DGame();
		break;
	}

	case GameState::Playing: {
		if (m_gameMode == GameMode::Shooter2D)
			Update2D(deltaTime, keyboardState);
		if (m_gameMode == GameMode::Arena3D)
			Update3D(deltaTime, keyboardState, mouseState);
		break;
	}

	case GameState::GameOver: {
		if (m_keyboardTracker.pressed.R) {
			if (m_gameMode == GameMode::Shooter2D)
				Start2DGame();
			if (m_gameMode == GameMode::Arena3D)
				Start3DGame();
		}
		if (m_keyboardTracker.pressed.Back)
			ReturnToTitle();
		break;
	}

	default:
		break;
	}
}

void Game::Update2D(
	float deltaTime,
	const DirectX::Keyboard::State& keyboardState
) {
	// update objects
	m_player->Update(keyboardState, deltaTime, m_windowWidth, m_windowHeight);

	// spawn bullet only once when space changes from released to pressed
	if (m_keyboardTracker.pressed.Space) {
		// emplace_back constructs the object and push at the back
		m_bullets.emplace_back(m_player->GetBulletSpawnPosition());

		// sfx
		m_audioManager.PlayShoot();
	}

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

			// sfx
			m_audioManager.PlayDamage();

			if (--m_playerHp <= 0) {
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

				// sfx
				m_audioManager.PlayHit();

				break;
			}
		}
	}

	// player vs enemy collision
	for (Enemy& enemy : m_enemies) {
		if (!enemy.IsActive())
			continue;

		if (Intersects(m_player->GetBounds(), enemy.GetBounds())) {
			enemy.Destroy();

			// sfx
			m_audioManager.PlayDamage();

			// if the player is invincible, skip the damage
			if (m_player->IsInvincible()) continue;

			// end the game
			if (--m_playerHp <= 0) {
				m_playerHp = 0;
				m_gameState = GameState::GameOver;
			}
			else m_player->StartInvincibility();
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
}

void Game::Update3D(
	float deltaTime,
	const DirectX::Keyboard::State& keyboardState,
	const DirectX::Mouse::State& mouseState
) {
	// rotate player3D with mouse movement
	const float yawDelta = static_cast<float>(mouseState.x) * MouseSensitivity;
	m_player3D.RotateYaw(-yawDelta);

	// update player3D
	m_player3D.Update(keyboardState, deltaTime);

	// fire bullet3D
	if (m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
		m_bullets3D.emplace_back(
			m_player3D.GetBulletSpawnPosition(),
			m_player3D.GetForwardDirection()
		);

		m_audioManager.PlayShoot();
	}

	// update camera to follow player
	m_cam.FollowBehind(
		m_player3D.GetPosition(),
		m_player3D.GetForwardDirection()
	);

	// spawning enemies3D
	m_enemy3DSpawnTimer += deltaTime;
	if (m_enemy3DSpawnTimer >= Enemy3DSpawnInterval) {
		m_enemy3DSpawnTimer -= Enemy3DSpawnInterval;
		SpawnEnemy3D();
	}

	// update enemy3D
	for (Enemy3D& enemy : m_enemies3D)
		enemy.Update(m_player3D.GetPosition(), deltaTime);

	// update bullet3D
	for (Bullet3D& bullet : m_bullets3D)
		bullet.Update(deltaTime);

	// Bullet3D vs Enemy3D collision
	for (Bullet3D& bullet : m_bullets3D) {
		if (!bullet.IsActive()) continue;

		for (Enemy3D& enemy : m_enemies3D) {
			if (!enemy.IsActive()) continue;

			if (Intersects(bullet.GetBounds(), enemy.GetBounds())) {
				bullet.Destroy();
				enemy.Destroy();

				m_score3D += 100;
				m_audioManager.PlayHit();
				break;
			}
		}
	}

	// Enemy3D vs Player3D collision
	for (Enemy3D& enemy : m_enemies3D) {
		if (!enemy.IsActive()) continue;

		if (Intersects(enemy.GetBounds(), m_player3D.GetBounds())) {
			enemy.Destroy();

			// if the player is invincible, skip the damage
			if (m_player3D.IsInvincible()) continue;

			m_audioManager.PlayDamage();

			// end the game
			if (--m_player3DHp <= 0) {
				m_player3DHp = 0;
				m_gameState = GameState::GameOver;
			}
			else m_player3D.StartInvincibility();
		}
	}

	m_enemies3D.erase(
		std::remove_if(
			m_enemies3D.begin(),
			m_enemies3D.end(),
			[](const Enemy3D& enemy) {
				return !enemy.IsActive();
			}
		),
		m_enemies3D.end()
	);

	m_bullets3D.erase(
		std::remove_if(
			m_bullets3D.begin(),
			m_bullets3D.end(),
			[](const Bullet3D& bullet) {
				return !bullet.IsActive();
			}
		),
		m_bullets3D.end()
	);
}

void Game::Render() {
	// bg color (just matching with the fog color, so fog could blend in
	const float clearColor[4] = {
		m_fogColor.x,
		m_fogColor.y,
		m_fogColor.z,
		1.0f
	};

	m_context->OMSetRenderTargets(
		1,
		m_renderTargetView.GetAddressOf(),
		m_depthStencilView.Get()
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

	// clear render target view
	m_context->ClearRenderTargetView(
		m_renderTargetView.Get(),
		clearColor
	);

	// clear depth stencil view
	m_context->ClearDepthStencilView(
		m_depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0
	);

	// ! render 3d
	if (m_gameMode == GameMode::Arena3D &&
		(m_gameState == GameState::Playing ||
			m_gameState == GameState::GameOver))
		Render3D();

	// ! render 2d
	m_spriteBatch->Begin();

	if (m_gameMode == GameMode::Shooter2D &&
		(m_gameState == GameState::Playing ||
			m_gameState == GameState::GameOver))
		Render2D();

	// draw ui
	DrawUI();

	m_spriteBatch->End();

	m_swapChain->Present(1, 0);
}

void Game::Render2D() {
	// draw enemies
	for (const Enemy& enemy : m_enemies)
		enemy.Draw(m_spriteBatch.get(), m_enemyTexture.Get());

	// drawing bullets before player, so that the player overlap the bullets
	for (const Bullet& bullet : m_bullets)
		bullet.Draw(m_spriteBatch.get(), m_bulletTexture.Get());

	// draw player
	m_player->Draw(m_spriteBatch.get());
}

void Game::Render3D() {
	const auto& view = m_cam.GetView();
	const auto& projection = m_cam.GetProjection();

	DirectX::BasicEffect* effect = m_basicEffect.get();
	ID3D11InputLayout* inputLayout = m_basicEffectInputLayout.Get();

	m_player3D.Draw(effect, inputLayout, view, projection);
	m_ground.Draw(effect, inputLayout, view, projection);
	for (const WallObject& wall : m_walls)
		wall.Draw(effect, inputLayout, view, projection);
	for (const Enemy3D& enemy : m_enemies3D)
		enemy.Draw(effect, inputLayout, view, projection);
	for (const Bullet3D& bullet : m_bullets3D)
		bullet.Draw(m_bullet3DPrimitive.get(), effect, inputLayout, view, projection);
}

void Game::Start2DGame() {
	m_gameMode = GameMode::Shooter2D;
	m_gameState = GameState::Playing;

	m_score = 0;
	m_playerHp = PLAYER_MAX_HP;

	m_bullets.clear();
	m_enemies.clear();

	m_enemySpawnTimer = 0.0f;

	if (m_player)
		m_player->Reset(m_windowWidth, m_windowHeight);
}

void Game::Start3DGame() {
	m_gameMode = GameMode::Arena3D;
	m_gameState = GameState::Playing;

	m_player3DHp = Player3DMaxHp;
	m_score3D = 0;

	m_enemies3D.clear();
	m_bullets3D.clear();

	m_enemy3DSpawnTimer = 0.0f;

	m_player3D.SetPosition(DirectX::SimpleMath::Vector3::Zero);

	// in relative mode, mouse only reports how much it moved this frame, not the actual screen position
	if (m_mouse)
		m_mouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	m_cam.FollowBehind(
		m_player3D.GetPosition(),
		m_player3D.GetForwardDirection()
	);
}

void Game::ReturnToTitle() {
	m_gameMode = GameMode::None;
	m_gameState = GameState::Title;

	m_bullets.clear();
	m_enemies.clear();

	m_bullets3D.clear();
	m_enemies3D.clear();

	m_enemySpawnTimer = 0.0f;
	m_enemy3DSpawnTimer = 0.0f;

	// reset mouse mode to absolute, so the mouse cursor can be used to click buttons
	if (m_mouse)
		m_mouse->SetMode(DirectX::Mouse::MODE_ABSOLUTE);
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

void Game::SpawnEnemy3D() {
	std::uniform_real_distribution<float> distribution(-8.0f, 8.0f);

	const float x = distribution(m_randomEngine);
	const float z = distribution(m_randomEngine);

	DirectX::SimpleMath::Vector3 spawnPos(x, 0.0f, z);

	Enemy3D enemy(spawnPos);

	enemy.Initialize(m_context.Get());

	m_enemies3D.emplace_back(std::move(enemy));	// (transfer the ownership) Because Enemy3D owns a unique_ptr<GeometricPrimitive>, it cannot be copied. It must be moved
}

void Game::DrawUI() {
	if (!m_font) return;

	using DirectX::SimpleMath::Vector2;

	switch (m_gameState) {
	case GameState::Title: {
		// title
		m_font->DrawString(
			m_spriteBatch.get(),
			L"DXTK GAME PROJECT",
			Vector2(
				m_windowWidth * 0.5f - 150.0f,
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
			const std::wstring scoreText = L"Score : " + std::to_wstring(m_score);
			const std::wstring hpText = L"HP    : " + std::to_wstring(m_playerHp);

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
			const std::wstring score3DText = L"3D Score : " + std::to_wstring(m_score3D);
			const std::wstring hp3DText = L"HP       : " + std::to_wstring(m_player3DHp);

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
		}
		break;
	}

	case GameState::GameOver: {
		std::wstring finalScoreText;

		if (m_gameMode == GameMode::Shooter2D)
			finalScoreText = L"Final Score:" + std::to_wstring(m_score);
		else if (m_gameMode == GameMode::Arena3D)
			finalScoreText = L"Final Score:" + std::to_wstring(m_score3D);

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