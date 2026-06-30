#include "WallObject.h"

#include <DirectXColors.h>

#include <stdexcept>

void WallObject::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("WallObject::Initialize received a null context");

	// create a cube, then scale it into a flat floor
	m_primitive = DirectX::GeometricPrimitive::CreateCube(context, 1.0f);
}

void WallObject::Draw(
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

	effect->SetDiffuseColor(Vector3(0.45f, 0.45f, 0.50f));
	effect->SetEmissiveColor(Vector3(0.02f, 0.02f, 0.025f));

	m_primitive->Draw(effect, inputLayout);
}

void WallObject::SetTransform(
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Vector3& scale
) noexcept {
	m_position = position;
	m_scale = scale;
}
