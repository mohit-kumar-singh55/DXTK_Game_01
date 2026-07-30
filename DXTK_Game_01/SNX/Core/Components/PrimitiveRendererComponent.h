#pragma once

#include <SNX/Core/Components/RendererComponent.h>

#include <DirectXMath.h>
#include <GeometricPrimitive.h>

#include <d3d11.h>

#include <memory>

enum class PrimitiveShape { Cube, Sphere };

class PrimitiveRendererComponent final : public RendererComponent {
public:
	PrimitiveRendererComponent(
		GameObject& gameObject,
		ID3D11DeviceContext* deviceContext,
		PrimitiveShape shape = PrimitiveShape::Cube
	) noexcept;

	void SetColor(const DirectX::XMVECTORF32& color) { m_color = color; }

	[[nodiscard]]
	const DirectX::XMVECTORF32& GetColor() const noexcept { return m_color; }

protected:
	void OnInitialize() override;

	void Draw(const RenderContext& context) override;

	void OnDestroy() override;

private:
	ID3D11DeviceContext* m_creationContext = nullptr;

	PrimitiveShape m_shape = PrimitiveShape::Cube;

	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::XMVECTORF32 m_color{ 1.0f, 1.0f, 1.0f, 1.0f };
};