#include "Camera.h"

void Camera::SetPerspective(
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

void Camera::SetOrthographic(
	float width,
	float height,
	float nearPlane,
	float farPlane
) noexcept {
	m_projection = DirectX::SimpleMath::Matrix::CreateOrthographic(
		width,
		height,
		nearPlane,
		farPlane
	);
}

void Camera::LookAt(
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Vector3& target,
	const DirectX::SimpleMath::Vector3& up
) noexcept {
	m_position = position;

	m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
		position,
		target,
		up
	);
}