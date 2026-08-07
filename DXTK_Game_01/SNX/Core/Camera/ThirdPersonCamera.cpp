#include "ThirdPersonCamera.h"

#include <algorithm>
#include <cmath>

void ThirdPersonCamera::Update(float deltaTime) noexcept {
	if (m_shakeTimer <= 0.0f) return;

	m_shakeTimer = (std::max)(0.0f, m_shakeTimer - deltaTime);
	m_shakeTime += deltaTime;
}

void ThirdPersonCamera::StartShake(float duration, float strength) noexcept {
	m_shakeDuration = (std::max)(0.0f, duration);
	m_shakeTimer = m_shakeDuration;
	m_shakeStrength = (std::max)(0.0f, strength);

	// restart the shake waveform
	m_shakeTime = 0.0f;
}

void ThirdPersonCamera::Follow(const DirectX::SimpleMath::Vector3& targetPosition) noexcept {
	const auto cameraPosition = targetPosition + m_followOffset;
	LookAt(cameraPosition, targetPosition);
}

void ThirdPersonCamera::FollowBehind(
	const DirectX::SimpleMath::Vector3& targetPosition,
	const DirectX::SimpleMath::Vector3& forwardDirection
) noexcept {
	using DirectX::SimpleMath::Vector3;

	// forward dir
	Vector3 forward = forwardDirection;
	forward.y = 0.0f;	// ignore y component

	if (forward.LengthSquared() <= 0.0001f)
		forward = Vector3(0.0f, 0.0f, -1.0f);	// default forward direction

	forward.Normalize();

	// right dir
	Vector3 right = forward.Cross(Vector3::Up);

	if (right.LengthSquared() <= 0.0001f)
		right = Vector3(1.0f, 0.0f, 0.0f);

	right.Normalize();

	const float cameraDistance = m_followOffset.Length();
	const float cameraHeight = m_followOffset.y;
	const float lookAheadDistance = 4.0f;	// how far ahead to look

	// set camera behind the player
	Vector3 cameraPos =
		targetPosition
		- forward * cameraDistance
		+ Vector3(0.0f, cameraHeight, 0.0f);

	// set camera to look slightly in front of the player
	Vector3 cameraTarget =
		targetPosition
		+ forward * lookAheadDistance
		+ Vector3(0.0f, 1.0f, 0.0f);	// look slightly above the target

	// camera shake
	if (m_shakeTimer > 0.0f && m_shakeDuration > 0.0f) {
		const float lifeRate = m_shakeTimer / m_shakeDuration;
		const float shakePower = m_shakeStrength * lifeRate;

		const float shakeX = std::sin(m_shakeTime * 85.0f) * shakePower;
		const float shakeY = std::sin(m_shakeTime * 120.0f) * shakePower * 0.6f;

		const Vector3 shakeOffset = right * shakeX + Vector3::Up * shakeY;

		cameraPos += shakeOffset;
		cameraTarget += shakeOffset;
	}

	LookAt(cameraPos, cameraTarget);
}