#pragma once

#include <Effects.h>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>

class Explosion3D {
public:
	explicit Explosion3D(const DirectX::SimpleMath::Vector3& position) noexcept;

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

	static constexpr float Duration = 0.35f;
};