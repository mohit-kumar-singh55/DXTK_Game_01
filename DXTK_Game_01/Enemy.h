#pragma once

#include <Core/Collision.h>

#include <d3d11.h>

#include <SimpleMath.h>
#include <SpriteBatch.h>

class Enemy {
public:
	static constexpr int TextureWidth = 64;
	static constexpr int TextureHeight = 64;

	explicit Enemy(const DirectX::SimpleMath::Vector2& spawnPos) noexcept;

	void Update(float deltaTime) noexcept;

	void Draw(
		DirectX::SpriteBatch* spriteBatch,
		ID3D11ShaderResourceView* texture
	) const;

	[[nodiscard]]
	bool IsOutsideScreen(int screenHeight) const noexcept;

	[[nodiscard]]
	bool IsActive() const noexcept;

	void Destroy() noexcept;

	[[nodiscard]]
	RectF GetBounds() const noexcept;

private:
	DirectX::SimpleMath::Vector2 m_pos;

	bool m_isActive = true;

	static constexpr float Speed = 120.0f;
};