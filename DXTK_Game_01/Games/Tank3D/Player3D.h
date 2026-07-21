#pragma once

#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <Keyboard.h>
#include <Effects.h>

#include <d3d11.h>
#include <memory>

#include <SNX/Core/Collision.h>

class Player3D {
public:
	void Initialize(ID3D11DeviceContext* context);

	void Update(
		const DirectX::Keyboard::State& keyboardState,
		float deltaTime
	);

	void Draw(
		DirectX::BasicEffect* effect,
		ID3D11InputLayout* inputLayout,
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

	[[nodiscard]]
	SphereBounds GetBounds() const noexcept;

	[[nodiscard]]
	bool IsInvincible() const noexcept;

	void StartInvincibility() noexcept;

	[[nodiscard]]
	DirectX::SimpleMath::Vector3 GetForwardDirection() const noexcept;

	[[nodiscard]]
	DirectX::SimpleMath::Vector3 GetBulletSpawnPosition() const noexcept;

	//void SetAimDirection(const DirectX::SimpleMath::Vector3& aimDirection) noexcept;

	void RotateTurretYaw(float radians) noexcept;

	[[nodiscard]]
	float GetBodyYaw() const noexcept;

	[[nodiscard]]
	float GetTurretYaw() const noexcept;

	[[nodiscard]]
	DirectX::SimpleMath::Vector3 GetBodyForwardDir() const noexcept;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;
	DirectX::SimpleMath::Vector3 m_forwardDir{ 0.0f, 0.0f, -1.0f };

	float m_bodyYaw = 0.0f;
	float m_turretYaw = 0.0f;
	DirectX::SimpleMath::Vector3 m_bodyForwardDir{ 0.0f, 0.0f, -1.0f };

	float m_minX = -9.0f;
	float m_maxX = 9.0f;
	float m_minZ = -9.0f;
	float m_maxZ = 9.0f;

	float m_invincibleTimer = 0.0f;

	static constexpr float MoveSpeed = 4.0f;
	static constexpr float BodyTurnSpeed = 2.5f;
	static constexpr float CollisionRadius = 1.0f;
	static constexpr float InvincibleDuration = 1.5f;
	static constexpr float BulletSpawnDistance = 3.0f;
	static constexpr float BulletSpawnHeight = 0.8f;
};