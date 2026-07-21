#pragma once

#include <Effects.h>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>

#include <d3d11.h>

#include <memory>

class DamageFlash3D {
public:
	void Initialize(ID3D11DeviceContext* context);

	void Update(float deltaTime) noexcept;

	void Draw(
		DirectX::BasicEffect* effect,
		ID3D11InputLayout* inputLayout,
		const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	void Trigger() noexcept;

	[[nodiscard]]
	bool IsActive() const noexcept;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	float m_timer = 0.0f;

	static constexpr float m_duration = 0.25f;
};