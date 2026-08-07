#pragma once

#include <SimpleMath.h>

class Camera {
public:
	Camera() = default;
	virtual ~Camera() = default;

	Camera(const Camera&) = default;
	Camera& operator=(const Camera&) = default;
	Camera(Camera&&) = default;
	Camera& operator=(Camera&&) = default;

	/*
	* Override only when a particular camera has
	* frame-dependent behaviour.
	*
	* A completely static Camera can use the
	* default implementation.
	*/
	virtual void Update(float deltaTime) noexcept {}

	void SetPerspective(
		float fovDegrees,
		float aspectRatio,
		float nearPlane,
		float farPlane
	) noexcept;

	void SetOrthographic(
		float width,
		float height,
		float nearPlane,
		float farPlane
	) noexcept;

	/*
	* Sets both the camera's world position
	* and its view matrix.
	*/
	void LookAt(
		const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Vector3& target,
		const DirectX::SimpleMath::Vector3& up = DirectX::SimpleMath::Vector3::Up
	) noexcept;

	[[nodiscard]]
	const DirectX::SimpleMath::Matrix& GetView() const noexcept { return m_view; }

	[[nodiscard]]
	const DirectX::SimpleMath::Matrix& GetProjection() const noexcept { return m_projection; }

	[[nodiscard]]
	const DirectX::SimpleMath::Vector3& GetPosition() const noexcept { return m_position; }

protected:
	/*
	* Used by specialised camera types that
	* calculate their own complete view matrix.
	*
	* Position is supplied separately so the
	* Camera's state stays consistent.
	*/
	void SetView(
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Vector3& position
	) noexcept {
		m_view = view;
		m_position = position;
	}

private:
	DirectX::SimpleMath::Matrix m_view = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix m_projection = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;
};