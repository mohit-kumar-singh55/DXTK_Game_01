#include "PrimitiveRendererComponent.h"

#include <SNX/Core/Components/Transform.h>
#include <SNX/Graphics/RenderContext.h>

#include <stdexcept>

PrimitiveRendererComponent::PrimitiveRendererComponent(
	GameObject& gameObject,
	ID3D11DeviceContext* deviceContext,
	PrimitiveShape shape
) noexcept :
	RendererComponent(gameObject),
	m_creationContext(deviceContext),
	m_shape(shape) {}

void PrimitiveRendererComponent::OnInitialize() {
	if (!m_creationContext)
		throw std::runtime_error("PrimitiveRendererComponent requires a valid device context");

	switch (m_shape) {
	case PrimitiveShape::Sphere:
		m_primitive = DirectX::GeometricPrimitive::CreateSphere(m_creationContext, 1.0f);
		break;
	case PrimitiveShape::Cube:
	default:
		m_primitive = DirectX::GeometricPrimitive::CreateCube(m_creationContext, 1.0f);
		break;
	}

	if (!m_primitive)
		throw std::runtime_error("Failed to create geometric primitive");
}

void PrimitiveRendererComponent::Draw(const RenderContext& context) {
	if (!m_primitive || !context.IsValid())
		return;

	m_primitive->Draw(
		GetTransform().GetWorldMatrix(),
		context.view,
		context.projection,
		m_color
	);
}

void PrimitiveRendererComponent::OnDestroy() {
	m_primitive.reset();
	m_creationContext = nullptr;
}