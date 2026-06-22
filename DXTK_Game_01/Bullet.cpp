#include "Bullet.h"
#include <DirectXColors.h>

Bullet::Bullet(const DirectX::SimpleMath::Vector2& spawnPos) noexcept {
	// spawn pos represents the center-bottom of the bullet
	// convert it to the top-left pos expected by SpriteBatch
	m_pos.x = spawnPos.x - static_cast<float>(TextureWidth) * 0.5f;
	m_pos.y = spawnPos.y - static_cast<float>(TextureHeight);
}

void Bullet::Update(float deltaTime) noexcept {
	// bullet going upwards
	m_pos.y -= Speed * deltaTime;
}

void Bullet::Draw(DirectX::SpriteBatch* spriteBatch, ID3D11ShaderResourceView* texture) const {
	if (!spriteBatch || !texture)
		return;

	spriteBatch->Draw(texture, m_pos, DirectX::Colors::White);
}

bool Bullet::IsOutsideScreen() const noexcept {
	return m_pos.y + static_cast<float>(TextureHeight) < 0.0f;
}

bool Bullet::IsActive() const noexcept {
	return m_isActive;
}

void Bullet::Destroy() noexcept {
	m_isActive = false;
}

RectF Bullet::GetBounds() const noexcept {
	return RectF{
		m_pos.x,
		m_pos.y,
		static_cast<float>(TextureWidth),
		static_cast<float>(TextureHeight),
	};
}