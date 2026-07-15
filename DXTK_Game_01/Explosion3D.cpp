#include "Explosion3D.h"

Explosion3D::Explosion3D(const DirectX::SimpleMath::Vector3& position) noexcept :
	m_position(position) {}

void Explosion3D::Update(float deltaTime) noexcept {
	m_timer += deltaTime;
}

void Explosion3D::Draw(
	DirectX::GeometricPrimitive* primitive,
	DirectX::BasicEffect* effect,
	ID3D11InputLayout* inputLayout,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!primitive || !effect || !inputLayout || !IsActive())
		return;

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const float lifeRate = m_timer / Duration;
	const float scale = 0.3f + lifeRate * 1.8f;

	const Matrix world =
		Matrix::CreateScale(Vector3::One * scale) *
		Matrix::CreateTranslation(m_position + Vector3(0.0f, 0.6f, 0.0f));

	effect->SetWorld(world);
	effect->SetView(view);
	effect->SetProjection(projection);

	// bright orange to darker red
	effect->SetDiffuseColor(Vector3(
		1.0f,
		0.45f * (1.0f - lifeRate),
		0.05f
	));

	effect->SetEmissiveColor(Vector3(
		0.8f * (1.0f - lifeRate),
		0.25f * (1.0f - lifeRate),
		0.02f
	));

	primitive->Draw(effect, inputLayout);
}

bool Explosion3D::IsActive() const noexcept {
	return m_timer < Duration;
}
