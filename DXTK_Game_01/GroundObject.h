#pragma once

#include <GeometricPrimitive.h>
#include <SimpleMath.h>

#include <d3d11.h>

#include <memory>

class GroundObject {
public:
	void Initialize(ID3D11DeviceContext* context);

	void Draw(
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3(0.0f, -1.05f, 0.0f);
	DirectX::SimpleMath::Vector3 m_scale = DirectX::SimpleMath::Vector3(20.0f, 0.1f, 20.0f);
};