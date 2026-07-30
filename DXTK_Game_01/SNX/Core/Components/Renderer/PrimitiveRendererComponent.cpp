#include "PrimitiveRendererComponent.h"

#include <SNX/Core/Components/Transform.h>
#include <SNX/Graphics/RenderContext.h>

#include <utility>
#include <stdexcept>

PrimitiveRendererComponent::PrimitiveRendererComponent(
	GameObject& gameObject,
	ID3D11DeviceContext* deviceContext,
	PrimitiveShape shape,
	std::shared_ptr<BasicPrimitiveMaterial> material
) noexcept :
	RendererComponent(gameObject),
	m_creationContext(deviceContext),
	m_shape(shape),
	m_material(std::move(material)) {}

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

	const auto& world = GetTransform().GetWorldMatrix();

	// ! use a shared custom material when one is supplied
	if (m_material) {
		m_material->Draw(
			*m_primitive,
			world,
			context,
			m_color,
			m_emissiveColor,
			m_wireframe
		);

		return;
	}

	m_primitive->Draw(
		world,
		context.view,
		context.projection,
		m_color,
		nullptr,
		m_wireframe
	);
}

void PrimitiveRendererComponent::OnDestroy() {
	m_primitive.reset();
	m_material.reset();

	m_creationContext = nullptr;
}