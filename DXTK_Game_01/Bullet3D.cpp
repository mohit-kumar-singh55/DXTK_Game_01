#include "Bullet3D.h"

#include <DirectXColors.h>

Bullet3D::Bullet3D(
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Vector3& direction
) noexcept :
	m_position(position),
	m_direction(direction)
{
	if (m_direction.LengthSquared() > 0.0001f)
		m_direction.Normalize();
	else
		m_direction = DirectX::SimpleMath::Vector3::Forward;
}

void Bullet3D::Update(float deltaTime) noexcept {
	if (!m_isActive) return;

	m_position += m_direction * Speed * deltaTime;

	m_lifeTime += deltaTime;
	if (m_lifeTime >= MaxLifeTime)
		m_isActive = false;
}

void Bullet3D::Draw(
	DirectX::GeometricPrimitive* primitive,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!primitive || !m_isActive)
		return;

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const Matrix world =
		Matrix::CreateScale(Vector3(0.25f, 0.25f, 0.25f)) *
		Matrix::CreateTranslation(m_position);

	primitive->Draw(
		world,
		view,
		projection,
		DirectX::Colors::Yellow
	);
}

SphereBounds Bullet3D::GetBounds() const noexcept {
	return SphereBounds{ m_position, CollisionRadius };
}

bool Bullet3D::IsActive() const noexcept {
	return m_isActive;
}

bool Bullet3D::IsExpired() const noexcept {
	return m_lifeTime >= MaxLifeTime;
}

void Bullet3D::Destroy() noexcept {
	m_isActive = false;
}
