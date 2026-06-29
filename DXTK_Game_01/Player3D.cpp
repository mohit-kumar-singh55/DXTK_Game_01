#include "Player3D.h"

#include <DirectXColors.h>

#include <algorithm>
#include <stdexcept>

void Player3D::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("Player3D::Initialize received a null context");

	m_primitive = DirectX::GeometricPrimitive::CreateCube(context, 2.0f);
}

void Player3D::Update(
	const DirectX::Keyboard::State& keyboardState,
	float deltaTime
) {
	// invincibility
	if (m_invincibleTimer > 0.0f)
		m_invincibleTimer -= deltaTime;

	// movement & rotation
	using DirectX::SimpleMath::Vector3;

	Vector3 move = Vector3::Zero;

	if (keyboardState.I) move.z -= 1.0f;
	if (keyboardState.K) move.z += 1.0f;
	if (keyboardState.J) move.x -= 1.0f;
	if (keyboardState.L) move.x += 1.0f;

	if (move.LengthSquared() > 0.0f) {
		move.Normalize();
		m_position += move * MoveSpeed * deltaTime;

		// setting forward dir
		m_forwardDir = move;

		// face the movement dir
		//m_yaw = std::atan2(move.x, -move.z);
		float newRot = std::atan2(-move.x, -move.z);
		m_yaw = std::lerp(m_yaw, newRot, deltaTime * 10);
	}

	// clamp cube inside the predifined area
	m_position.x = std::clamp(m_position.x, m_minX, m_maxX);
	m_position.z = std::clamp(m_position.z, m_minZ, m_maxZ);
}

void Player3D::Draw(
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive) return;

	// flicker during invincibility
	if (m_invincibleTimer > 0.0f) {
		const int flickerFrame = static_cast<int>(m_invincibleTimer * 20.0f);
		if (flickerFrame % 2 == 0) return;
	}

	using DirectX::SimpleMath::Matrix;

	const Matrix world =
		Matrix::CreateRotationY(m_yaw) *
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

void Player3D::SetPosition(const DirectX::SimpleMath::Vector3& position) noexcept {
	m_position = position;
}

DirectX::SimpleMath::Vector3 Player3D::GetPosition() const noexcept {
	return m_position;
}

void Player3D::SetMovementBounds(
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

SphereBounds Player3D::GetBounds() const noexcept {
	return SphereBounds{ m_position,CollisionRadius };
}

bool Player3D::IsInvincible() const noexcept {
	return m_invincibleTimer > 0.0f;
}

void Player3D::StartInvincibility() noexcept {
	m_invincibleTimer = InvincibleDuration;
}

DirectX::SimpleMath::Vector3 Player3D::GetForwardDirection() const noexcept {
	return m_forwardDir;
}

DirectX::SimpleMath::Vector3 Player3D::GetBulletSpawnPosition() const noexcept {
	return m_position +
		m_forwardDir * 1.4f +
		DirectX::SimpleMath::Vector3(0.0f, 0.2f, 0.0f);
}
