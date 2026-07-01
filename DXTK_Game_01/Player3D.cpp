#include "Player3D.h"

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

	if (keyboardState.W || keyboardState.Up)
		move.z -= 1.0f;
	if (keyboardState.S || keyboardState.Down)
		move.z += 1.0f;
	if (keyboardState.A || keyboardState.Left)
		move.x -= 1.0f;
	if (keyboardState.D || keyboardState.Right)
		move.x += 1.0f;

	if (move.LengthSquared() > 0.0f) {
		move.Normalize();
		m_position += move * MoveSpeed * deltaTime;

		// ! now rotation is handled by the aim direction, so we don't need to rotate the player based on movement anymore
		/*
			// setting forward dir
			m_forwardDir = move;

			// face the movement dir
			// ! rotating through the short way (Ex: instead of going 350 to 10 by substracting 340, going 350 to 370 and substrct 360)
			float targetRot = std::atan2(-move.x, -move.z);
			float diff = targetRot - m_yaw;
			// wrap to [-PI, PI]
			diff = std::remainder(diff, DirectX::XM_2PI);
			m_yaw += diff * std::clamp(deltaTime * 10.0f, 0.0f, 1.0f);
		*/
	}

	// clamp cube inside the predifined area
	m_position.x = std::clamp(m_position.x, m_minX, m_maxX);
	m_position.z = std::clamp(m_position.z, m_minZ, m_maxZ);
}

void Player3D::Draw(
	DirectX::BasicEffect* effect,
	ID3D11InputLayout* inputLayout,
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
	using DirectX::SimpleMath::Vector3;

	const Matrix world =
		Matrix::CreateRotationY(m_yaw) *
		Matrix::CreateTranslation(m_position);

	effect->SetWorld(world);
	effect->SetView(view);
	effect->SetProjection(projection);

	effect->SetDiffuseColor(Vector3(0.2f, 0.45f, 1.0f));
	effect->SetEmissiveColor(Vector3(0.02f, 0.04f, 0.08f));

	m_primitive->Draw(effect, inputLayout);
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

void Player3D::SetAimDirection(const DirectX::SimpleMath::Vector3& aimDirection) noexcept {
	DirectX::SimpleMath::Vector3 aimDir = aimDirection;

	aimDir.y = 0.0f; // ignore y component for rotation

	if (aimDir.LengthSquared() <= 0.0001f)
		return;

	aimDir.Normalize();
	m_forwardDir = aimDir;

	// rotate the player to face the aim direction
	m_yaw = std::atan2(aimDir.x, aimDir.z);
}