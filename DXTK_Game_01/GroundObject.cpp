#include "GroundObject.h"

#include <DirectXColors.h>

#include <stdexcept>

void GroundObject::Initialize(ID3D11DeviceContext* context) {
	if (!context)
		throw std::invalid_argument("GroundObject::Initialize received a null context");

	// create a cube, then scale it into a flat floor
	m_primitive = DirectX::GeometricPrimitive::CreateCube(context, 1.0f);
}

void GroundObject::Draw(
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive) return;

	using DirectX::SimpleMath::Matrix;

	const Matrix world = Matrix::CreateScale(m_scale) * Matrix::CreateTranslation(m_position);

	m_primitive->Draw(world, view, projection, DirectX::Colors::DarkSeaGreen);
}
