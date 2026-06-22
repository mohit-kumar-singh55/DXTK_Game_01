#include "Enemy.h"

Enemy::Enemy(const DirectX::SimpleMath::Vector2& spawnPos) noexcept :
	m_pos(spawnPos)
{}

void Enemy::Update(float deltaTime) noexcept {
	m_pos.y += Speed * deltaTime;
}

void Enemy::Draw(DirectX::SpriteBatch* spriteBatch, ID3D11ShaderResourceView* texture) const {
	if (!spriteBatch || !texture || !m_isActive)
		return;

	spriteBatch->Draw(texture, m_pos, DirectX::Colors::White);
}

bool Enemy::IsOutsideScreen(int screenHeight) const noexcept {
	return m_pos.y > static_cast<float>(screenHeight);
}

bool Enemy::IsActive() const noexcept {
	return m_isActive;
}

void Enemy::Destroy() noexcept {
	m_isActive = false;
}

RectF Enemy::GetBounds() const noexcept {
	return RectF{
		m_pos.x,
		m_pos.y,
		static_cast<float>(TextureWidth),
		static_cast<float>(TextureHeight),
	};
}