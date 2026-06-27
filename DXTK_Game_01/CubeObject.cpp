#include "CubeObject.h"

#include <DirectXColors.h>

#include <algorithm>
#include <stdexcept>

void CubeObject::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("CubeObject::Initialize received a null context");

	m_primitive = DirectX::GeometricPrimitive::CreateCube(context, 2.0f);
}

void CubeObject::Update(
	const DirectX::Keyboard::State& keyboardState,
	float deltaTime
) {
	using DirectX::SimpleMath::Vector3;

	Vector3 move = Vector3::Zero;

	if (keyboardState.I) move.z -= 1.0f;
	if (keyboardState.K) move.z += 1.0f;
	if (keyboardState.J) move.x -= 1.0f;
	if (keyboardState.L) move.x += 1.0f;

	if (move.LengthSquared() > 0.0f) {
		move.Normalize();
		m_position += move * MoveSpeed * deltaTime;
	}

	// clamp cube inside the predifined area
	m_position.x = std::clamp(m_position.x, m_minX, m_maxX);
	m_position.z = std::clamp(m_position.z, m_minZ, m_maxZ);

	m_rotation += deltaTime;
}

void CubeObject::Draw(
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive) return;

	using DirectX::SimpleMath::Matrix;

	const Matrix world =
		Matrix::CreateRotationY(m_rotation) *
		Matrix::CreateRotationX(m_rotation * 0.5f) *
		Matrix::CreateTranslation(m_position);

	/*
	* draw cube
	* using this object transform (world)
	* using this camera (view)
	* using this perspective lens (projection)
	* with this color
	*/
	m_primitive->Draw(
		world,
		view,
		projection,
		DirectX::Colors::CornflowerBlue
	);
}

void CubeObject::SetPosition(const DirectX::SimpleMath::Vector3& position) noexcept {
	m_position = position;
}

DirectX::SimpleMath::Vector3 CubeObject::GetPosition() const noexcept {
	return m_position;
}

void CubeObject::SetMovementBounds(
	float minX,
	float maxX,
	float minZ,
	float maxZ
) noexcept {
	m_minX = minX;
	m_maxX = maxX;
	m_minZ = minZ;
	m_maxZ = maxZ;
}