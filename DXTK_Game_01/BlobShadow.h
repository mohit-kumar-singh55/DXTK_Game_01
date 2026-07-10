#pragma once

#include <Effects.h>
#include <GeometricPrimitive.h>
#include <SimpleMath.h>

#include <d3d11.h>

#include <memory>

class BlobShadow {
public:
	void Initialize(ID3D11DeviceContext* context);

	void Draw(
		DirectX::BasicEffect* effect,
		ID3D11InputLayout* inputLayout,
		const DirectX::SimpleMath::Vector3& position,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

private:
	std::unique_ptr < DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_scale = DirectX::SimpleMath::Vector3(2.8f, 0.02f, 4.0f);

	float m_groundY = -0.97f;
};