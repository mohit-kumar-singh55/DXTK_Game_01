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

	RECT destinationRec = {};

	// forcing the texture to fit inside the rect with the size as we want
	destinationRec.left = static_cast<LONG>(m_pos.x);
	destinationRec.top = static_cast<LONG>(m_pos.y);
	destinationRec.right = static_cast<LONG>(m_pos.x + TextureWidth);
	destinationRec.bottom = static_cast<LONG>(m_pos.y + TextureHeight);

	spriteBatch->Draw(texture, destinationRec, DirectX::Colors::White);
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