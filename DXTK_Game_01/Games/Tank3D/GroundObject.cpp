#include "GroundObject.h"

#include <stdexcept>

void GroundObject::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("GroundObject::Initialize received a null context");

	// create a cube, then scale it into a flat floor
	m_primitive = DirectX::GeometricPrimitive::CreateCube(context, 1.0f);
}

void GroundObject::CreateInputLayout(
	DirectX::IEffect* effect,
	ID3D11InputLayout** inputLayout
) const {
	if (!m_primitive || !effect || !inputLayout)
		return;

	m_primitive->CreateInputLayout(effect, inputLayout);
}

void GroundObject::Draw(
	DirectX::BasicEffect* effect,
	ID3D11InputLayout* inputLayout,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive) return;

	using DirectX::SimpleMath::Matrix;
	using DirectX::SimpleMath::Vector3;

	const Matrix world = Matrix::CreateScale(m_scale) * Matrix::CreateTranslation(m_position);

	effect->SetWorld(world);
	effect->SetView(view);
	effect->SetProjection(projection);

	effect->SetDiffuseColor(Vector3(0.25f, 0.45f, 0.25f));
	effect->SetEmissiveColor(Vector3(0.02f, 0.04f, 0.02f));

	m_primitive->Draw(effect, inputLayout);
}
