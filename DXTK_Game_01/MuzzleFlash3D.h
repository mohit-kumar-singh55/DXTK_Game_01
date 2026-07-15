#pragma once

#include <Effects.h>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>

#include <d3d11.h>

#include <memory>

class MuzzleFlash3D {
public:
	void Initialize(ID3D11DeviceContext* context);

	void Update(float deltaTime) noexcept;

	void Trigger(
		const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Vector3& direction
	) noexcept;

	void Draw(
		DirectX::BasicEffect* effect,
		ID3D11InputLayout* inputLayout,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	[[nodiscard]]
	bool IsActive() const noexcept;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;

	float m_yaw = 0.0f;
	float m_timer = 0.0f;

	static constexpr float m_duration = 0.08f;
};