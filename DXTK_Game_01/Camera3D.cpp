#include "Camera3D.h"

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

const DirectX::SimpleMath::Matrix& Camera3D::GetView() const noexcept {
	return m_view;
}

const DirectX::SimpleMath::Matrix& Camera3D::GetProjection() const noexcept {
	return m_projection;
}
