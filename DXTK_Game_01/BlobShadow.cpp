#include "BlobShadow.h"

#include <stdexcept>

void BlobShadow::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("BlobShadow: Initialize received a null context.");

	m_primitive = DirectX::GeometricPrimitive::CreateSphere(context, 1.0);
}

void BlobShadow::Draw(
	DirectX::BasicEffect* effect,
	ID3D11InputLayout* inputLayout,
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive || !effect || !inputLayout)
		return;

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	Vector3 shadowPos = position;
	shadowPos.y = m_groundY;

	const Matrix world =
		Matrix::CreateScale(m_scale) *
		Matrix::CreateTranslation(shadowPos);

	effect->SetWorld(world);
	effect->SetView(view);
	effect->SetProjection(projection);

	// dark color, no transparency 
	effect->SetDiffuseColor(Vector3(0.02f, 0.02f, 0.02f));
	effect->SetEmissiveColor(Vector3::Zero);

	m_primitive->Draw(effect, inputLayout);
}