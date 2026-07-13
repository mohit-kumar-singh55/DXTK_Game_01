#pragma once

#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <Effects.h>

#include <d3d11.h>
#include <memory>

#include "Collision.h"

class Enemy3D {
public:
	explicit Enemy3D(const DirectX::SimpleMath::Vector3& position) noexcept;

	void Initialize(ID3D11DeviceContext* context);

	void Update(
		const DirectX::SimpleMath::Vector3& playerPosition,
		float deltaTime
	);

	void Draw(
		DirectX::BasicEffect* effect,
		ID3D11InputLayout* inputLayout,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	[[nodiscard]]
	DirectX::SimpleMath::Vector3 GetPosition() const noexcept;

	[[nodiscard]]
	bool IsActive() const noexcept;

	void Destroy() noexcept;

	[[nodiscard]]
	SphereBounds GetBounds() const noexcept;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_position;

	bool m_isActive = true;

	static constexpr float MoveSpeed = 2.0f;
	static constexpr float CollisionRadius = 0.75f;
};