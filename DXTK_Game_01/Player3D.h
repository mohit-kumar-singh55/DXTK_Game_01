#pragma once

#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <Keyboard.h>

#include <d3d11.h>
#include <memory>

class Player3D {
public:
	void Initialize(ID3D11DeviceContext* context);

	void Update(
		const DirectX::Keyboard::State& keyboardState,
		float deltaTime
	);

	void Draw(
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	void SetPosition(const DirectX::SimpleMath::Vector3& position) noexcept;

	[[nodiscard]]
	DirectX::SimpleMath::Vector3 GetPosition() const noexcept;

	void SetMovementBounds(
		float minX,
		float maxX,
		float minZ,
		float maxZ
	) noexcept;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;
	float m_yaw = 0.0f;

	float m_minX = -9.0f;
	float m_maxX = 9.0f;
	float m_minZ = -9.0f;
	float m_maxZ = 9.0f;

	static constexpr float MoveSpeed = 4.0f;
};