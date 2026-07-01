#pragma once

#include <SimpleMath.h>

class Camera3D {
public:
	void SetPerspective(
		float fovDegrees,
		float aspectRatio,
		float nearPlane,
		float farPlane
	) noexcept;

	void SetFollowOffset(const DirectX::SimpleMath::Vector3& offset) noexcept;

	void Follow(const DirectX::SimpleMath::Vector3& targetPosition) noexcept;

	// 3rd person camera that also rotates towards the forward dir
	void FollowBehind(
		const DirectX::SimpleMath::Vector3& targetPosition,
		const DirectX::SimpleMath::Vector3& forwardDirection
	) noexcept;

	[[nodiscard]]
	const DirectX::SimpleMath::Matrix& GetView() const noexcept;

	[[nodiscard]]
	const DirectX::SimpleMath::Matrix& GetProjection() const noexcept;

	[[nodiscard]]
	bool ScreenPointToGround(
		float screenX,
		float screenY,
		float screenWidth,
		float screenHeight,
		float groundY,
		DirectX::SimpleMath::Vector3& result
	) const noexcept;

private:
	DirectX::SimpleMath::Matrix m_view = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix m_projection = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Vector3 m_followOffset = DirectX::SimpleMath::Vector3(0.0f, 3.0f, 8.0f);
};