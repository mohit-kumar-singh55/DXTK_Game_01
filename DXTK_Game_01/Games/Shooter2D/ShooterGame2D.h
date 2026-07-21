#pragma once

#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"

#include <SpriteBatch.h>

#include <memory>
#include <random>
#include <vector>

class AudioManager;

class ShooterGame2D final {
public:
	void Initialize(
		ID3D11Device* device,
		int screenWidth,
		int screenHeight
	);

	void Start();
	void Clear();

	void Update(
		float deltaTime,
		AudioManager& audioManager
	);

	void Render(DirectX::SpriteBatch* spriteBatch) const;

	[[nodiscard]]
	bool IsGameOver() const noexcept { return m_isGameOver; }

	[[nodiscard]]
	int GetScore() const noexcept { return m_score; }

	[[nodiscard]]
	int GetPlayerHP() const noexcept { return m_playerHp; }

private:
	void SpawnEnemy();

	void CreateBulletTexture(ID3D11Device* device);

	void CreateEnemyTexture(ID3D11Device* device);

	void DamagePlayer(AudioManager& audioManager);

private:
	int m_screenWidth = 0;
	int m_screenHeight = 0;

	std::unique_ptr<Player> m_player;

	std::vector<Bullet> m_bullets;
	std::vector<Enemy> m_enemies;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_bulletTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_enemyTexture;

	float m_enemySpawnTimer = 0.0f;

	int m_score = 0;
	int m_playerHp = PlayerMaxHP;

	bool m_isGameOver = false;

	std::mt19937 m_randomEngine
	{
		std::random_device{}()
	};

	static constexpr float EnemySpawnInterval = 1.0f;
	static constexpr int PlayerMaxHP = 3;
};