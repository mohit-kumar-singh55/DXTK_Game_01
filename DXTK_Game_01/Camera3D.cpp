#include "Camera3D.h"

#include <cmath>

void Camera3D::SetPerspective(
	float fovDegrees,
	float aspectRatio,
	float nearPlane,
	float farPlane
) noexcept {
	m_projection = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
		DirectX::XMConvertToRadians(fovDegrees),
		aspectRatio,
		nearPlane,
		farPlane
	);
}

void Camera3D::Update(float deltaTime) noexcept {
	if (m_shakeTimer > 0.0f) {
		m_shakeTimer -= deltaTime;
		m_shakeTime += deltaTime;
	}
}

void Camera3D::StartShake(float duration, float strength) noexcept {
	m_shakem_duration = duration;
	m_shakeTimer = duration;
	m_shakeStrength = strength;
}

void Camera3D::SetFollowOffset(const DirectX::SimpleMath::Vector3& offset) noexcept {
	m_followOffset = offset;
}

void Camera3D::Follow(const DirectX::SimpleMath::Vector3& targetPosition) noexcept {
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const Vector3 cameraPos = targetPosition + m_followOffset;

	m_view = Matrix::CreateLookAt(
		cameraPos,
		targetPosition,				// look at target
		Vector3::Up
	);
}

void Camera3D::FollowBehind(
	const DirectX::SimpleMath::Vector3& targetPosition,
	const DirectX::SimpleMath::Vector3& forwardDirection
) noexcept {
	using DirectX::SimpleMath::Matrix;
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
	if (m_shakeTimer > 0.0f && m_shakem_duration > 0.0f) {
		const float lifeRate = m_shakeTimer / m_shakem_duration;
		const float shakePower = m_shakeStrength * lifeRate;

		const float shakeX = std::sin(m_shakeTime * 85.0f) * shakePower;
		const float shakeY = std::sin(m_shakeTime * 120.0f) * shakePower * 0.6f;

		const Vector3 shakeOffset = right * shakeX + Vector3::Up * shakeY;

		cameraPos += shakeOffset;
		cameraTarget += shakeOffset;
	}

	m_view = Matrix::CreateLookAt(
		cameraPos,
		cameraTarget,
		Vector3::Up
	);
}

const DirectX::SimpleMath::Matrix& Camera3D::GetView() const noexcept {
	return m_view;
}

const DirectX::SimpleMath::Matrix& Camera3D::GetProjection() const noexcept {
	return m_projection;
}

// raycasting from mouse's screen position to the ground plane (y = groundY)
bool Camera3D::ScreenPointToGround(
	float screenX,
	float screenY,
	float screenWidth,
	float screenHeight,
	float groundY,
	DirectX::SimpleMath::Vector3& result
) const noexcept {
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const Matrix world = Matrix::Identity;

	// unproject the screen point to world space (near point)
	const DirectX::XMVECTOR nearPoint = DirectX::XMVector3Unproject(
		DirectX::XMVectorSet(screenX, screenY, 0.0f, 1.0f),
		0.0f, 0.0f, screenWidth, screenHeight,
		0.0f, 1.0f,
		m_projection, m_view, world
	);

	// unproject the screen point to world space (far point)
	const DirectX::XMVECTOR farPoint = DirectX::XMVector3Unproject(
		DirectX::XMVectorSet(screenX, screenY, 1.0f, 1.0f),
		0.0f, 0.0f, screenWidth, screenHeight,
		0.0f, 1.0f,
		m_projection, m_view, world
	);

	Vector3 rayStart, rayEnd;

	// store the unprojected points into Vector3
	DirectX::XMStoreFloat3(&rayStart, nearPoint);
	DirectX::XMStoreFloat3(&rayEnd, farPoint);

	Vector3 rayDir = rayEnd - rayStart;

	// ray direction is too small
	if (rayDir.LengthSquared() <= 0.0001f)
		return false;

	// normalize the ray direction
	rayDir.Normalize();

	// ray is parallel to the ground plane
	if (std::abs(rayDir.y) <= 0.0001f)
		return false;

	// calculate the intersection point with the ground plane
	const float t = (groundY - rayStart.y) / rayDir.y;

	// intersection point is behind the camera
	if (t < 0.0f)
		return false;

	result = rayStart + rayDir * t;
	return true;
}
