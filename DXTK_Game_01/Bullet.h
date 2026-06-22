#pragma once

#include "Collision.h"

#include <d3d11.h>

#include <SimpleMath.h>
#include <SpriteBatch.h>

// ! all bullets will share one GPU texture
class Bullet {
public:
	static constexpr int TextureWidth = 12;
	static constexpr int TextureHeight = 24;

	explicit Bullet(const DirectX::SimpleMath::Vector2& spawnPos) noexcept;

	void Update(float deltaTime) noexcept;

	void Draw(
		DirectX::SpriteBatch* spriteBatch,
		ID3D11ShaderResourceView* texture
	) const;

	[[nodiscard]]
	bool IsOutsideScreen() const noexcept;

	[[nodiscard]]
	bool IsActive() const noexcept;

	void Destroy() noexcept;

	[[nodiscard]]
	RectF GetBounds() const noexcept;

private:
	DirectX::SimpleMath::Vector2 m_pos;

	bool m_isActive = true;

	static constexpr float Speed = 700.0f;
};