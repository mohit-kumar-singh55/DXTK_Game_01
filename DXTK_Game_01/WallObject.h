#pragma once

#include <GeometricPrimitive.h>
#include <SimpleMath.h>

#include <d3d11.h>

#include <memory>

class WallObject {
public:
	void Initialize(ID3D11DeviceContext* context);

	void Draw(
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	void SetTransform(
		const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Vector3& scale
	) noexcept;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;
	DirectX::SimpleMath::Vector3 m_scale = DirectX::SimpleMath::Vector3::One;
};