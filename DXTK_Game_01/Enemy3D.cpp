#include "Enemy3D.h"

#include <DirectXColors.h>

#include <stdexcept>

Enemy3D::Enemy3D(
	const DirectX::SimpleMath::Vector3& position
) noexcept : m_position(position)
{}

void Enemy3D::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("Enemy3D::Initialize received a null context");

	m_primitive = DirectX::GeometricPrimitive::CreateSphere(context, 1.0f);
}

void Enemy3D::Update(
	const DirectX::SimpleMath::Vector3& playerPosition,
	float deltaTime
) {
	if (!m_isActive) return;

	using DirectX::SimpleMath::Vector3;

	// move towards player
	Vector3 dir = playerPosition - m_position;
	dir.y = 0.0f;

	if (dir.LengthSquared() > 0.0001f) {
		dir.Normalize();
		m_position += dir * MoveSpeed * deltaTime;
	}
}

void Enemy3D::Draw(
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive) return;

	using DirectX::SimpleMath::Matrix;

	const Matrix world = Matrix::CreateTranslation(m_position);

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
		DirectX::Colors::IndianRed
	);
}

DirectX::SimpleMath::Vector3 Enemy3D::GetPosition() const noexcept {
	return m_position;
}

bool Enemy3D::IsActive() const noexcept {
	return m_isActive;
}

void Enemy3D::Destroy() noexcept {
	m_isActive = false;
}

SphereBounds Enemy3D::GetBounds() const noexcept {
	return SphereBounds{ m_position,CollisionRadius };
}