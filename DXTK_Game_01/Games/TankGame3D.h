#pragma once

#include <Camera3D.h>
#include <Player3D.h>
#include <GroundObject.h>
#include <WallObject.h>
#include <Bullet3D.h>
#include <AudioManager.h>
#include <Enemy3D.h>
#include <TankVisual.h>
#include <BlobShadow.h>
#include <Explosion3D.h>
#include <MuzzleFlash3D.h>
#include <DamageFlash3D.h>

#include <CommonStates.h>
#include <Effects.h>
#include <GeometricPrimitive.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <SimpleMath.h>

#include <wrl/client.h>

#include <array>
#include <memory>
#include <random>
#include <vector>

#include <d3d11.h>

class TankGame3D final {
public:
	void Initialize(
		ID3D11Device* device,
		ID3D11DeviceContext* context,
		int windowWidth,
		int windowHeight
	);

	void Start();
	void Clear();

	void Update(
		float deltaTime,
		AudioManager& audioManager
	);

	void Render();

	[[nodiscard]]
	bool IsGameOver() const noexcept { return m_isGameOver; }

	[[nodiscard]]
	bool IsTankDestroyed() const noexcept { return m_isTankDestroyed; }

	[[nodiscard]]
	int GetScore() const noexcept { return m_score; }

	[[nodiscard]]
	int GetPlayerHP() const noexcept { return m_playerHp; }

	[[nodiscard]]
	const DirectX::SimpleMath::Vector3& GetClearColor() const noexcept { return m_fogColor; }

private:
	void InitializeBasicEffect(ID3D11Device* device);

	void SpawnEnemy();

	void DestroyTank(
		AudioManager& audioManager
	);

	void DrawEnemyHealthBar(
		const Enemy3D& enemy,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection,
		const DirectX::SimpleMath::Vector3& cameraPosition
	) const;

private:
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;

	Camera3D m_cam;
	Player3D m_player;

	GroundObject m_ground;
	std::array<WallObject, 4> m_walls;

	std::vector<Enemy3D> m_enemies;
	std::vector<Bullet3D> m_bullets;
	std::vector<Explosion3D> m_explosions;

	std::unique_ptr<DirectX::GeometricPrimitive>
		m_bulletPrimitive;

	std::unique_ptr<DirectX::GeometricPrimitive>
		m_explosionPrimitive;

	std::unique_ptr<DirectX::GeometricPrimitive>
		m_healthBarPrimitive;

	std::unique_ptr<DirectX::BasicEffect> m_basicEffect;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>
		m_basicEffectInputLayout;

	std::unique_ptr<DirectX::CommonStates>
		m_commonStates;

	std::unique_ptr<DirectX::EffectFactory>
		m_modelEffectFactory;

	TankVisual m_tankVisual;
	BlobShadow m_playerShadow;
	MuzzleFlash3D m_muzzleFlash;
	DamageFlash3D m_damageFlash;

	DirectX::SimpleMath::Vector3 m_fogColor
	{
		1.0f,
		1.0f,
		1.0f
	};

	float m_fogStart = 6.0f;
	float m_fogEnd = 18.0f;

	float m_enemySpawnTimer = 0.0f;

	int m_playerHp = 3;
	int m_score = 0;

	bool m_isTankDestroyed = false;
	bool m_isGameOver = false;

	float m_tankDeathTimer = 0.0f;

	std::mt19937 m_randomEngine
	{
		std::random_device{}()
	};

	static constexpr float MouseSensitivity = 0.0035f;

	static constexpr float EnemySpawnInterval = 2.0f;
	static constexpr float EnemySpawnMinDistanceFromPlayer = 6.0f;
	static constexpr float EnemySpawnEdge = 8.5f;

	static constexpr int PlayerMaxHp = 3;

	static constexpr float TankDeathGameOverDelay = 1.4f;
};