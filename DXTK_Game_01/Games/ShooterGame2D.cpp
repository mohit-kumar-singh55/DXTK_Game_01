#include <Games/ShooterGame2D.h>

#include <Audio/AudioManager.h>
#include <Core/Collision.h>
#include <Core/TextureFactory.h>

#include <algorithm>
#include <stdexcept>

#include <Input/InputManager.h>

void ShooterGame2D::Initialize(
	ID3D11Device* device,
	int screenWidth,
	int screenHeight
) {
	if (!device)
		throw std::invalid_argument("ShooterGame2D requires a valid device.");

	if (screenWidth <= 0 || screenHeight <= 0)
		throw std::invalid_argument("ShooterGame2D requires a valid screen size.");

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	m_player = std::make_unique<Player>();
	m_player->Initialize(device);

	// create shared textures
	CreateBulletTexture(device);
	CreateEnemyTexture(device);

	Start();
}

void ShooterGame2D::Start() {
	m_score = 0;
	m_playerHp = PlayerMaxHP;

	m_bullets.clear();
	m_enemies.clear();

	m_enemySpawnTimer = 0.0f;
	m_isGameOver = false;

	if (m_player)
		m_player->Reset(m_screenWidth, m_screenHeight);
}

void ShooterGame2D::Clear() {
	m_bullets.clear();
	m_enemies.clear();

	m_enemySpawnTimer = 0.0f;
	m_isGameOver = false;
}

void ShooterGame2D::Update(
	float deltaTime,
	AudioManager& audioManager
) {
	auto& input = InputManager::Get();

	const auto& keyboardState = input.GetKeyboardState();

	// update objects
	m_player->Update(keyboardState, deltaTime, m_screenWidth, m_screenHeight);

	// spawn bullet only once
	if (input.IsKeyPressed(DirectX::Keyboard::Space)) {
		// emplace_back constructs the object and push at the back
		m_bullets.emplace_back(m_player->GetBulletSpawnPosition());

		// sfx
		audioManager.PlayShoot();
	}

	// update bullets
	for (Bullet& bullet : m_bullets)
		bullet.Update(deltaTime);

	// spawn enemies
	m_enemySpawnTimer += deltaTime;
	if (m_enemySpawnTimer >= EnemySpawnInterval) {
		m_enemySpawnTimer -= EnemySpawnInterval;
		SpawnEnemy();
	}

	// update enemies
	for (Enemy& enemy : m_enemies)
		enemy.Update(deltaTime);

	// enemy reaching bottom damages the player
	for (Enemy& enemy : m_enemies) {
		if (!enemy.IsActive())
			continue;

		if (enemy.IsOutsideScreen(m_screenHeight)) {
			enemy.Destroy();
			DamagePlayer(audioManager);
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
				bullet.Destroy();
				enemy.Destroy();
				m_score += 100;

				// sfx
				audioManager.PlayHit();

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

			// if the player is invincible, skip the damage
			if (m_player->IsInvincible()) continue;

			DamagePlayer(audioManager);

			if (!m_isGameOver)
				m_player->StartInvincibility();
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

void ShooterGame2D::Render(DirectX::SpriteBatch* spriteBatch) const {
	// draw enemies
	for (const Enemy& enemy : m_enemies)
		enemy.Draw(spriteBatch, m_enemyTexture.Get());

	// drawing bullets before player, so that the player overlap the bullets
	for (const Bullet& bullet : m_bullets)
		bullet.Draw(spriteBatch, m_bulletTexture.Get());

	// draw player
	m_player->Draw(spriteBatch);
}

void ShooterGame2D::SpawnEnemy() {
	const float maximumX = static_cast<float>(m_screenWidth - Enemy::TextureWidth);

	// Generate random float values between 0.0f and WINDOW_WIDTH - Enemy::TextureWidth
	std::uniform_real_distribution<float> distribution(
		0.0f,
		maximumX > 0.0f ? maximumX : 0.0f
	);

	const float x = distribution(m_randomEngine);

	DirectX::SimpleMath::Vector2 spawnPos{
		x,
		-static_cast<float>(Enemy::TextureHeight)
	};

	m_enemies.emplace_back(spawnPos);
}

void ShooterGame2D::CreateBulletTexture(ID3D11Device* device) {
	// actual image texture
	try {
		m_bulletTexture = TextureFactory::LoadTextureFromFile(
			device,
			L"Assets/Textures/bullet.png"
		);
	}
	// fallback debug texture
	catch (...) {
		m_bulletTexture = TextureFactory::CreateRectangleTexture(
			device,
			Bullet::TextureWidth,
			Bullet::TextureHeight,
			TextureFactory::ColorRGBA{ 255, 210, 50, 255 },
			TextureFactory::ColorRGBA{ 255, 255, 255, 255 },
			1
		);
	}
}

void ShooterGame2D::CreateEnemyTexture(ID3D11Device* device) {
	// actual image texture
	try {
		m_enemyTexture = TextureFactory::LoadTextureFromFile(
			device,
			L"Assets/Textures/enemy.png"
		);
	}
	// fallback debug texture
	catch (...) {
		m_enemyTexture = TextureFactory::CreateRectangleTexture(
			device,
			Enemy::TextureWidth,
			Enemy::TextureHeight,
			TextureFactory::ColorRGBA{ 255, 80, 100, 255 },
			TextureFactory::ColorRGBA{ 255, 255, 255, 255 },
			4
		);
	}
}

void ShooterGame2D::DamagePlayer(AudioManager& audioManager) {
	if (m_isGameOver) return;

	// sfx
	audioManager.PlayDamage();

	if (--m_playerHp <= 0) {
		m_playerHp = 0;
		m_isGameOver = true;
	}
}