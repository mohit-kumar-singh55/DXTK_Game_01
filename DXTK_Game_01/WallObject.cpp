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
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	if (!m_primitive) return;

	using DirectX::SimpleMath::Matrix;

	const Matrix world = Matrix::CreateScale(m_scale) * Matrix::CreateTranslation(m_position);

	m_primitive->Draw(world, view, projection, DirectX::Colors::SlateGray);
}

void WallObject::SetTransform(
	const DirectX::SimpleMath::Vector3& position,
	const DirectX::SimpleMath::Vector3& scale
) noexcept {
	m_position = position;
	m_scale = scale;
}
