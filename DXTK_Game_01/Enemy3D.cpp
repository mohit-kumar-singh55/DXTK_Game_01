#include "Enemy3D.h"

#include <stdexcept>

Enemy3D::Enemy3D(
	const DirectX::SimpleMath::Vector3& position,
	Enemy3DType type
) noexcept :
	m_position(position),
	m_type(type) {
	ApplyTypeSettings();
}

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

	if (m_hitFlashTimer > 0.0f)
		m_hitFlashTimer -= deltaTime;

	using DirectX::SimpleMath::Vector3;

	// move towards player
	Vector3 dir = playerPosition - m_position;
	dir.y = 0.0f;

	if (dir.LengthSquared() > 0.0001f) {
		dir.Normalize();
		m_position += dir * m_moveSpeed * deltaTime;
	}
}

void Enemy3D::Draw(
	DirectX::BasicEffect* effect,
	ID3D11InputLayout* inputLayout,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive || !m_isActive || !effect || !inputLayout) return;

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const Matrix world =
		Matrix::CreateScale(Vector3::One * m_visualScale) *
		Matrix::CreateTranslation(m_position);

	effect->SetWorld(world);
	effect->SetView(view);
	effect->SetProjection(projection);

	// damage flash
	if (m_hitFlashTimer > 0.0f) {
		effect->SetDiffuseColor(Vector3(1.0f, 1.0f, 1.0f));
		effect->SetEmissiveColor(Vector3(0.5f, 0.5f, 0.5f));
	}
	else {
		effect->SetDiffuseColor(m_diffuseColor);
		effect->SetEmissiveColor(m_emissiveColor);
	}

	m_primitive->Draw(effect, inputLayout);
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
	return SphereBounds{ m_position, m_collisionRadius };
}

int Enemy3D::GetScoreValue() const noexcept {
	return m_scorevalue;
}

void Enemy3D::TakeDamage(int damage) noexcept {
	if (!IsActive()) return;

	m_hp -= damage;
	m_hitFlashTimer = HitFlashDuration;

	if (m_hp > 0) return;

	// destroy if no hp remaining
	m_hp = 0;
	Destroy();
}

bool Enemy3D::IsDead() const noexcept {
	return m_hp <= 0;
}

int Enemy3D::GetHP() const noexcept {
	return m_hp;
}

void Enemy3D::ApplyTypeSettings() noexcept {
	using DirectX::SimpleMath::Vector3;

	switch (m_type) {
	case Enemy3DType::Normal:
		m_moveSpeed = 2.0f;
		m_collisionRadius = 0.75f;
		m_visualScale = m_collisionRadius * 2.0f;
		m_scorevalue = 100;

		m_maxHp = 1;
		m_hp = m_maxHp;

		m_diffuseColor = Vector3(1.0f, 0.2f, 0.15f);
		m_emissiveColor = Vector3(0.08f, 0.01f, 0.01f);
		break;

	case Enemy3DType::Fast:
		m_moveSpeed = 4.0f;
		m_collisionRadius = 0.95f;
		m_visualScale = m_collisionRadius * 2.0f;
		m_scorevalue = 150;

		m_maxHp = 1;
		m_hp = m_maxHp;

		m_diffuseColor = Vector3(1.0f, 0.9f, 0.1f);
		m_emissiveColor = Vector3(0.12f, 0.08f, 0.01f);
		break;

	case Enemy3DType::Heavy:
		m_moveSpeed = 1.2f;
		m_collisionRadius = 1.0f;
		m_visualScale = m_collisionRadius * 2.0f;
		m_scorevalue = 250;

		m_maxHp = 3;
		m_hp = m_maxHp;

		m_diffuseColor = Vector3(0.6f, 0.1f, 1.0f);
		m_emissiveColor = Vector3(0.06f, 0.01f, 0.12f);
		break;
	}
}