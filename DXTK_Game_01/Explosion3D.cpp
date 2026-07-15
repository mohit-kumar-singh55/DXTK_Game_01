#include "Explosion3D.h"

Explosion3D::Explosion3D(
	const DirectX::SimpleMath::Vector3& position,
	float duration,
	float maxScale,
	float heightOffset
) noexcept :
	m_position(position),
	m_duration(duration),
	m_maxScale(maxScale),
	m_heightOffset(heightOffset)
{}

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

	const float lifeRate = m_timer / m_duration;
	const float scale = 0.3f + lifeRate * m_maxScale;

	const Matrix world =
		Matrix::CreateScale(Vector3::One * scale) *
		Matrix::CreateTranslation(m_position + Vector3(0.0f, m_heightOffset, 0.0f));

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
	return m_timer < m_duration;
}
