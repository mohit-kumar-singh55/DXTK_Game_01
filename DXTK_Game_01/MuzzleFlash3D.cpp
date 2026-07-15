#include "MuzzleFlash3D.h"

#include <DirectXColors.h>

#include <cmath>
#include <stdexcept>

void MuzzleFlash3D::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("MuzzleFlash3D::Initialize received a null context");

	m_primitive = DirectX::GeometricPrimitive::CreateSphere(context, 1.0f);
}

void MuzzleFlash3D::Update(float deltaTime) noexcept {
	if (m_timer <= 0.0f) return;

	m_timer -= deltaTime;
}

void MuzzleFlash3D::Trigger(
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Vector3& direction
) noexcept {
	using DirectX::SimpleMath::Vector3;

	m_position = position;

	Vector3 forward = direction;
	forward.y = 0.0f;

	if (forward.LengthSquared() > 0.0001f) {
		forward.Normalize();
		m_yaw = std::atan2(forward.x, -forward.z);
	}

	m_timer = m_duration;
}

void MuzzleFlash3D::Draw(
	DirectX::BasicEffect* effect,
	ID3D11InputLayout* inputLayout,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive || !effect || !inputLayout || !IsActive())
		return;

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const float scale = 0.5f + m_timer * 8.0f;

	const Matrix world =
		Matrix::CreateScale(Vector3(scale, scale, scale * 1.6f)) *
		Matrix::CreateRotationY(m_yaw) *
		Matrix::CreateTranslation(m_position);

	effect->SetWorld(world);
	effect->SetView(view);
	effect->SetProjection(projection);

	effect->SetDiffuseColor(Vector3(1.0f, 0.65f, 0.05f));
	effect->SetEmissiveColor(Vector3(1.0f, 0.35f, 0.02f));

	m_primitive->Draw(effect, inputLayout);
}

bool MuzzleFlash3D::IsActive() const noexcept {
	return m_timer > 0.0f;
}
