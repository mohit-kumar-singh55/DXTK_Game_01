#pragma once

#include <Core/Collision.h>

#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <Effects.h>

class Bullet3D {
public:
	Bullet3D(
		const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Vector3& direction
	) noexcept;

	void Update(float deltaTime) noexcept;

	void Draw(
		DirectX::GeometricPrimitive* primitive,
		DirectX::BasicEffect* effect,
		ID3D11InputLayout* inputLayout,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	[[nodiscard]]
	SphereBounds GetBounds() const noexcept;

	[[nodiscard]]
	bool IsActive() const noexcept;

	[[nodiscard]]
	bool IsExpired() const noexcept;

	void Destroy() noexcept;

private:
	DirectX::SimpleMath::Vector3 m_position;
	DirectX::SimpleMath::Vector3 m_direction;

	float m_lifeTime = 0.0f;
	bool m_isActive = true;

	static constexpr float Speed = 12.0f;
	static constexpr float MaxLifeTime = 3.0f;
	static constexpr float CollisionRadius = 0.25f;
};