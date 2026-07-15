#include "DamageFlash3D.h"

#include <stdexcept>

void DamageFlash3D::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("DamageFlash3D::Initialize received a null context");

	m_primitive = DirectX::GeometricPrimitive::CreateSphere(context, 1.0f);
}

void DamageFlash3D::Update(float deltaTime) noexcept {
	if (m_timer <= 0.0f) return;
	m_timer -= deltaTime;
}

void DamageFlash3D::Draw(
	DirectX::BasicEffect* effect,
	ID3D11InputLayout* inputLayout,
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive || !effect || !inputLayout || !IsActive())
		return;

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const float lifeRate = m_timer / Duration;
	const float scale = 4.2f + lifeRate * 0.5f;

	const Matrix world =
		Matrix::CreateScale(Vector3(scale, scale * 0.65f, scale * 1.4f)) *
		Matrix::CreateTranslation(position + Vector3(0.0f, 0.7f, 0.0f));

	effect->SetWorld(world);
	effect->SetView(view);
	effect->SetProjection(projection);

	effect->SetDiffuseColor(Vector3(1.0f, 0.05f, 0.05f));
	effect->SetEmissiveColor(Vector3(0.8f * lifeRate, 0.02f, 0.02f));

	m_primitive->Draw(effect, inputLayout);
}

void DamageFlash3D::Trigger() noexcept {
	m_timer = Duration;
}

bool DamageFlash3D::IsActive() const noexcept {
	return m_timer > 0.0f;
}
