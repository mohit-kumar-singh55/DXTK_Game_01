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

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	// body forward dir
	if (keyboardState.A || keyboardState.Left)
		m_bodyYaw += BodyTurnSpeed * deltaTime;
	if (keyboardState.D || keyboardState.Right)
		m_bodyYaw -= BodyTurnSpeed * deltaTime;

	// rotate difauld forward dir vector by m_bodyYaw angle
	m_bodyForwardDir = Vector3::Transform(
		Vector3(0.0f, 0.0f, -1.0f),
		Matrix::CreateRotationY(m_bodyYaw)
	);

	m_bodyForwardDir.y = 0.0f;
	m_bodyForwardDir.Normalize();

	// move in the body forward dir
	Vector3 move = Vector3::Zero;

	if (keyboardState.W || keyboardState.Up)
		move += m_bodyForwardDir;
	if (keyboardState.S || keyboardState.Down)
		move -= m_bodyForwardDir;

	if (move.LengthSquared() > 0.0f) {
		move.Normalize();
		m_position += move * MoveSpeed * deltaTime;
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
		Matrix::CreateRotationY(m_bodyYaw) *
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
	m_invincibleTimer = Invinciblem_duration;
}

DirectX::SimpleMath::Vector3 Player3D::GetForwardDirection() const noexcept {
	return m_forwardDir;
}

DirectX::SimpleMath::Vector3 Player3D::GetBulletSpawnPosition() const noexcept {
	using DirectX::SimpleMath::Vector3;

	Vector3 forward = m_forwardDir;
	forward.y = 0.0f;

	if (forward.LengthSquared() <= 0.0001f)
		forward = Vector3(0.0f, 0.0f, -1.0f);

	forward.Normalize();

	return m_position +
		forward * BulletSpawnDistance +
		Vector3(0.0f, BulletSpawnHeight, 0.0f);
}

// mouse movement changes yaw
// yaw creates forward dir
// foward dir controls player facing, camera dir, and bullet dir
void Player3D::RotateTurretYaw(float radians) noexcept {
	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	m_turretYaw += radians;

	m_forwardDir = Vector3::Transform(
		Vector3(0.0f, 0.0f, -1.0f),
		Matrix::CreateRotationY(m_turretYaw)
	);

	m_forwardDir.y = 0.0f; // ensure forward dir is horizontal

	if (m_forwardDir.LengthSquared() > 0.0001f)
		m_forwardDir.Normalize();
}

float Player3D::GetBodyYaw() const noexcept {
	return m_bodyYaw;
}

float Player3D::GetTurretYaw() const noexcept {
	return m_turretYaw;
}

DirectX::SimpleMath::Vector3 Player3D::GetBodyForwardDir() const noexcept {
	return m_bodyForwardDir;
}
