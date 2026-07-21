#pragma once

#include <Effects.h>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>

class Explosion3D {
public:
	explicit Explosion3D(
		const DirectX::SimpleMath::Vector3& position,
		float duration = 0.35f,
		float maxScale = 1.8f,
		float heightOffset = 0.6f
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
	bool IsActive() const noexcept;

private:
	DirectX::SimpleMath::Vector3 m_position;

	float m_timer = 0.0f;

	float m_duration = 0.35f;
	float m_maxScale = 1.8f;
	float m_heightOffset = 0.6f;
};