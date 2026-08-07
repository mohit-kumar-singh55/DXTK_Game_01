#pragma once

#include "Camera.h"

class ThirdPersonCamera final : public Camera {
public:
	ThirdPersonCamera() = default;
	~ThirdPersonCamera() override = default;

	void Update(float deltaTime) noexcept override;

	void StartShake(float duration, float strength) noexcept;

	void SetFollowOffset(const DirectX::SimpleMath::Vector3& offset) noexcept { m_followOffset = offset; }

	[[nodiscard]]
	const DirectX::SimpleMath::Vector3& GetFollowOffset() const noexcept { return m_followOffset; }

	void Follow(const DirectX::SimpleMath::Vector3& targetPosition) noexcept;

	// 3rd person camera that also rotates towards the forward dir
	void FollowBehind(
		const DirectX::SimpleMath::Vector3& targetPosition,
		const DirectX::SimpleMath::Vector3& forwardDirection
	) noexcept;

private:
	DirectX::SimpleMath::Vector3 m_followOffset = DirectX::SimpleMath::Vector3(0.0f, 3.0f, 8.0f);

	float m_shakeTimer = 0.0f;
	float m_shakeDuration = 0.0f;
	float m_shakeStrength = 0.0f;
	float m_shakeTime = 0.0f;
};