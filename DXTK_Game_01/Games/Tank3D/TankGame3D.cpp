#include "TankGame3D.h"

#include <DirectXColors.h>

#include <stdexcept>
#include <algorithm>

#include <SNX/Input/InputManager.h>

void TankGame3D::Initialize(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	int windowWidth,
	int windowHeight
) {
	if (!device || !context)
		throw std::invalid_argument("TankGame3D requires a valid device and context");

	using DirectX::SimpleMath::Vector3;

	m_context = context;

	// create a cube
	m_player.Initialize(m_context.Get());
	m_player.SetPosition(Vector3::Zero);
	m_player.SetMovementBounds(-8.5f, 8.5f, -8.5f, 8.5f);

	// initialize bullet
	m_bulletPrimitive = DirectX::GeometricPrimitive::CreateSphere(
		m_context.Get(),
		1.0f
	);

	// initialize explosion shape
	m_explosionPrimitive = DirectX::GeometricPrimitive::CreateSphere(
		m_context.Get(),
		1.0f
	);

	// create ground
	m_ground.Initialize(m_context.Get());

	// walls
	for (WallObject& wall : m_walls)
		wall.Initialize(m_context.Get());

	// shared primitive for health bar
	m_healthBarPrimitive = DirectX::GeometricPrimitive::CreateCube(
		m_context.Get(),
		1.0f
	);

	// initialize basic effect
	InitializeBasicEffect(device);

	// initialize common states & effect factory
	m_commonStates = std::make_unique<DirectX::CommonStates>(device);
	m_modelEffectFactory = std::make_unique<DirectX::EffectFactory>(device);

	// setting 3d model texture directory
	m_modelEffectFactory->SetDirectory(L"Assets\\Models\\Runtime\\Tank\\textures");

	// ! creating tank model
	if (m_tankVisual.Load(device, *m_modelEffectFactory))
		m_tankVisual.UpdateEffects(m_fogColor, m_fogStart, m_fogEnd);

	// fake tank shadow
	m_playerShadow.Initialize(m_context.Get());

	// muzzle flash
	m_muzzleFlash.Initialize(m_context.Get());

	// damage flash
	m_damageFlash.Initialize(m_context.Get());

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
		static_cast<float>(windowWidth) /
		static_cast<float>(windowHeight);

	// 3D perspective lens (camera lens)
	m_cam.SetPerspective(60.0f, aspectRatio, 0.1f, 100.0f);

	// camera offset from the player
	m_cam.SetFollowOffset(Vector3(0.0f, 6.0f, 8.0f));

	// update camera
	m_cam.FollowBehind(
		m_player.GetPosition(),
		m_player.GetForwardDirection()
	);
}

void TankGame3D::Start() {
	m_playerHp = PlayerMaxHp;
	m_score = 0;

	m_isTankDestroyed = false;
	m_isGameOver = false;

	m_tankDeathTimer = 0.0f;
	m_enemySpawnTimer = 0.0f;

	m_enemies.clear();
	m_bullets.clear();
	m_explosions.clear();

	m_player.SetPosition(DirectX::SimpleMath::Vector3::Zero);

	m_cam.FollowBehind(
		m_player.GetPosition(),
		m_player.GetForwardDirection()
	);
}

void TankGame3D::Clear() {
	m_bullets.clear();
	m_enemies.clear();
	m_explosions.clear();

	m_enemySpawnTimer = 0.0f;
	m_enemySpawnTimer = 0.0f;

	m_isTankDestroyed = false;
	m_isGameOver = false;
}

void TankGame3D::Update(
	float deltaTime,
	AudioManager& audioManager
) {
	auto& input = InputManager::Get();

	const auto& keyboardState = input.GetKeyboardState();
	const auto& mouseState = input.GetMouseState();
	const auto& mouseTracker = input.GetMouseTracker();

	// update camera
	m_cam.Update(deltaTime);

	// ! if tank is destroyed, update necessary things only
	if (m_isTankDestroyed) {
		m_tankDeathTimer -= deltaTime;

		m_muzzleFlash.Update(deltaTime);
		m_damageFlash.Update(deltaTime);

		// update explosion
		for (Explosion3D& explosion : m_explosions)
			explosion.Update(deltaTime);

		m_explosions.erase(
			std::remove_if(
				m_explosions.begin(),
				m_explosions.end(),
				[](const Explosion3D& explosion) {
					return !explosion.IsActive();
				}
			),
			m_explosions.end()
		);

		// update camera to follow player
		m_cam.FollowBehind(
			m_player.GetPosition(),
			m_player.GetForwardDirection()
		);

		// move to gameover state, if all explosions are over
		if (m_tankDeathTimer <= 0.0f)
			//m_gameState = GameState::GameOver;
			m_isGameOver = true;

		return;
	}

	// rotate player3D with mouse movement
	const float yawDelta = static_cast<float>(mouseState.x) * MouseSensitivity;
	m_player.RotateTurretYaw(-yawDelta);

	// update player3D
	m_player.Update(keyboardState, deltaTime);

	// fire bullet3D
	if (mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::ButtonState::PRESSED) {
		const DirectX::SimpleMath::Vector3 bulletSpawnPos = m_player.GetBulletSpawnPosition();
		const DirectX::SimpleMath::Vector3 bulletDir = m_player.GetForwardDirection();

		// spawn bullet and shoot
		m_bullets.emplace_back(bulletSpawnPos, bulletDir);

		// show muzzle flash
		m_muzzleFlash.Trigger(bulletSpawnPos, bulletDir);

		// shake camera
		m_cam.StartShake(0.08f, 0.04f);

		// sfx
		audioManager.PlayShoot();
	}

	// update muzzle flash
	m_muzzleFlash.Update(deltaTime);

	// update damage flash
	m_damageFlash.Update(deltaTime);

	// spawning enemies3D
	m_enemySpawnTimer += deltaTime;
	if (m_enemySpawnTimer >= EnemySpawnInterval) {
		m_enemySpawnTimer -= EnemySpawnInterval;
		SpawnEnemy();
	}

	// update enemy3D
	for (Enemy3D& enemy : m_enemies)
		enemy.Update(m_player.GetPosition(), deltaTime);

	// update bullet3D
	for (Bullet3D& bullet : m_bullets)
		bullet.Update(deltaTime);

	// update explosion
	for (Explosion3D& explosion : m_explosions)
		explosion.Update(deltaTime);

	// Bullet3D vs Enemy3D collision
	for (Bullet3D& bullet : m_bullets) {
		if (!bullet.IsActive()) continue;

		for (Enemy3D& enemy : m_enemies) {
			if (!enemy.IsActive()) continue;

			if (Intersects(bullet.GetBounds(), enemy.GetBounds())) {
				bullet.Destroy();
				enemy.TakeDamage(1);

				audioManager.PlayHit();

				if (enemy.IsDead()) {
					// spawn explosion
					m_explosions.emplace_back(enemy.GetPosition());

					m_score += enemy.GetScoreValue();
				}

				break;
			}
		}
	}

	// Enemy3D vs Player3D collision
	for (Enemy3D& enemy : m_enemies) {
		if (!enemy.IsActive()) continue;

		if (Intersects(enemy.GetBounds(), m_player.GetBounds())) {
			enemy.Destroy();

			// if the player is invincible, skip the damage
			if (m_player.IsInvincible()) continue;

			// camera shake
			m_cam.StartShake(0.25f, 0.22f);

			// damage flash
			m_damageFlash.Trigger();

			// sfx
			audioManager.PlayDamage();

			// end the game
			if (--m_playerHp <= 0) {
				m_playerHp = 0;
				DestroyTank(audioManager);	// ! game over
			}
			else m_player.StartInvincibility();
		}
	}

	m_enemies.erase(
		std::remove_if(
			m_enemies.begin(),
			m_enemies.end(),
			[](const Enemy3D& enemy) {
				return !enemy.IsActive();
			}
		),
		m_enemies.end()
	);

	m_bullets.erase(
		std::remove_if(
			m_bullets.begin(),
			m_bullets.end(),
			[](const Bullet3D& bullet) {
				return !bullet.IsActive();
			}
		),
		m_bullets.end()
	);

	m_explosions.erase(
		std::remove_if(
			m_explosions.begin(),
			m_explosions.end(),
			[](const Explosion3D& explosion) {
				return !explosion.IsActive();
			}
		),
		m_explosions.end()
	);

	// update camera to follow player
	m_cam.FollowBehind(
		m_player.GetPosition(),
		m_player.GetForwardDirection()
	);
}

void TankGame3D::Render() {
	const auto& view = m_cam.GetView();
	const auto& projection = m_cam.GetProjection();

	DirectX::BasicEffect* effect = m_basicEffect.get();
	ID3D11InputLayout* inputLayout = m_basicEffectInputLayout.Get();

	if (!m_isTankDestroyed) {
		// render player model if available
		if (m_tankVisual.IsLoaded() && m_commonStates) {
			m_tankVisual.SetWorldTransform(
				m_player.GetPosition() + DirectX::SimpleMath::Vector3(0.0f, -1.0f, 0.0f),
				m_player.GetBodyYaw(),
				m_player.GetTurretYaw()
			);

			m_tankVisual.Draw(m_context.Get(), *m_commonStates, view, projection);
		}
		// render default cube
		else
			m_player.Draw(effect, inputLayout, view, projection);

		// draw tank's fake shadow
		m_playerShadow.Draw(effect, inputLayout, m_player.GetPosition(), view, projection);
	}

	m_ground.Draw(effect, inputLayout, view, projection);

	for (const WallObject& wall : m_walls)
		wall.Draw(effect, inputLayout, view, projection);
	for (const Enemy3D& enemy : m_enemies) {
		// draw enemy
		enemy.Draw(effect, inputLayout, view, projection);

		// draw enemy's health bar
		DrawEnemyHealthBar(enemy, view, projection, m_cam.GetPosition());
	}
	for (const Bullet3D& bullet : m_bullets)
		bullet.Draw(m_bulletPrimitive.get(), effect, inputLayout, view, projection);
	for (const Explosion3D& explosion : m_explosions)
		explosion.Draw(m_explosionPrimitive.get(), effect, inputLayout, view, projection);

	// draw muzzle flash
	m_muzzleFlash.Draw(effect, inputLayout, view, projection);

	// draw damage flash
	m_damageFlash.Draw(effect, inputLayout, m_player.GetPosition(), view, projection);
}

void TankGame3D::InitializeBasicEffect(ID3D11Device* device) {
	m_basicEffect = std::make_unique<DirectX::BasicEffect>(device);

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

void TankGame3D::SpawnEnemy() {
	using DirectX::SimpleMath::Vector3;

	std::uniform_real_distribution<float> posDistribution(-EnemySpawnEdge, EnemySpawnEdge);
	std::uniform_int_distribution<int> sideDistribution(0, 3);
	std::uniform_int_distribution<int> typeDistribution(0, 9);

	// decide type of enemy
	const int typeRoll = typeDistribution(m_randomEngine);

	Enemy3DType enemyType = Enemy3DType::Normal;

	if (typeRoll <= 5)
		enemyType = Enemy3DType::Normal;
	else if (typeRoll <= 7)
		enemyType = Enemy3DType::Fast;
	else
		enemyType = Enemy3DType::Heavy;

	// calc spawn pos
	const Vector3 playerPos = m_player.GetPosition();

	Vector3 spawnPos = Vector3::Zero;

	const int MaxAttempts = 20;

	for (int attempt = 0;attempt < MaxAttempts;attempt++) {
		const int side = sideDistribution(m_randomEngine);
		const float randomVal = posDistribution(m_randomEngine);

		switch (side) {
		case 0:
			// back edge
			spawnPos = Vector3(randomVal, 0.0f, -EnemySpawnEdge);
			break;
		case 1:
			// front edge
			spawnPos = Vector3(randomVal, 0.0f, EnemySpawnEdge);
			break;
		case 2:
			// left edge
			spawnPos = Vector3(-EnemySpawnEdge, 0.0f, randomVal);
			break;
		case 3:
		default:
			// right edge
			spawnPos = Vector3(EnemySpawnEdge, 0.0f, randomVal);
			break;
		}

		// check if enemy spawn pos is far enough from player
		const float distanceSquared = Vector3::DistanceSquared(spawnPos, playerPos);
		const float minDistanceSquared =
			EnemySpawnMinDistanceFromPlayer *
			EnemySpawnMinDistanceFromPlayer;

		if (distanceSquared >= minDistanceSquared)
			break;
	}

	Enemy3D enemy(spawnPos, enemyType);

	enemy.Initialize(m_context.Get());

	m_enemies.emplace_back(std::move(enemy));	// (transfer the ownership) Because Enemy3D owns a unique_ptr<GeometricPrimitive>, it cannot be copied. It must be moved
}

void TankGame3D::DestroyTank(AudioManager& audioManager) {
	if (m_isTankDestroyed) return;

	using DirectX::SimpleMath::Vector3;

	m_isTankDestroyed = true;
	m_tankDeathTimer = TankDeathGameOverDelay;

	const Vector3 tankPos = m_player.GetPosition();

	// spawn multiple explosions
	m_explosions.emplace_back(
		tankPos,
		1.2f,
		4.0f,
		0.8f
	);
	m_explosions.emplace_back(
		tankPos + Vector3(0.8f, 0.0f, 0.4f),
		0.8f,
		2.5f,
		0.7f
	);
	m_explosions.emplace_back(
		tankPos + Vector3(-0.7f, 0.0f, -0.5f),
		0.9f,
		2.2f,
		0.7f
	);

	// shake camera
	m_cam.StartShake(0.6f, 0.45f);

	// sfx
	audioManager.PlayDamage();

	// clean up
	m_enemies.clear();
	m_bullets.clear();
}

void TankGame3D::DrawEnemyHealthBar(
	const Enemy3D& enemy,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection,
	const DirectX::SimpleMath::Vector3& cameraPosition
) const {
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	if (!m_healthBarPrimitive || !enemy.ShouldShowHealthBar())
		return;

	constexpr float totalWidth = 1.4f;
	constexpr float barHeight = 0.14f;
	constexpr float barDepth = 0.04f;
	constexpr float heightOffset = 1.8f;

	const float healthRatio = enemy.GetHealthRatio();

	// position the bar above the enemy
	const Vector3 barPos = enemy.GetPosition() + Vector3(0.0f, heightOffset, 0.0f);

	// rotate the bar so that it always faces the cmaera (billboarding)
	const Matrix billboard = Matrix::CreateBillboard(barPos, cameraPosition, Vector3::Up);

	// draw the black bg
	const Matrix bgWorld = Matrix::CreateScale(totalWidth, barHeight, barDepth) * billboard;

	m_healthBarPrimitive->Draw(bgWorld, view, projection, DirectX::Colors::Black);

	// current width of the health portion
	const float currentWidth = totalWidth * healthRatio;

	// moving the pivot to the left
	const float leftOffset = (totalWidth - currentWidth) * 0.5f;

	// move the colored part slightly towards the camera
	Vector3 dirToCam = cameraPosition - barPos;
	dirToCam.Normalize();
	const Vector3 fillPos = barPos + dirToCam * 0.03f;

	// draw filled part
	const Matrix fillBillboard = Matrix::CreateBillboard(fillPos, cameraPosition, Vector3::Up);

	const Matrix fillWorld =
		Matrix::CreateScale(currentWidth, barHeight * 0.65f, barDepth) *
		Matrix::CreateTranslation(leftOffset, 0.0f, 0.0f) *
		fillBillboard;

	DirectX::XMVECTORF32 fillColor = DirectX::Colors::Green;

	if (healthRatio <= 0.34f)
		fillColor = DirectX::Colors::Red;
	else if (healthRatio <= 0.67f)
		fillColor = DirectX::Colors::Orange;

	m_healthBarPrimitive->Draw(fillWorld, view, projection, fillColor);
}