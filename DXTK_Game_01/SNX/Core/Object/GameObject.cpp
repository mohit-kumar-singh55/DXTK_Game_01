#include "GameObject.h"

#include <algorithm>

GameObject::GameObject(std::string name) :
	m_name(name) {

}

GameObject::~GameObject() {
	DestroyComponents();
}

void GameObject::Initialize() {
	if (m_initialized) return;

	for (const auto& component : m_components) {
		if (component->m_initialized)
			continue;

		component->OnInitialize();
		component->m_initialized = true;
	}

	m_initialized = true;
}

void GameObject::EnsureComponentStarted(Component& component) {
	if (!component.m_enabled || component.m_started || component.m_removeRequested)
		return;

	component.OnStart();
	component.m_started = true;
}

void GameObject::RemoveRequestedComponents() noexcept {
	std::erase_if(
		m_components,
		[](const std::unique_ptr<Component>& component) {
			if (!component) return true;
			if (!component->m_removeRequested)
				return false;

			// if remove is requested but has not destryoed yet
			// so first call ondestroy of the component and then remove it
			if (!component->m_destroyed) {
				component->OnDestroy();
				component->m_destroyed = true;
			}

			return true;
		}
	);
}

void GameObject::FixedUpdate() {
	if (!m_active || m_destroyRequested)
		return;

	Initialize();

	for (const auto& component : m_components) {
		if (!component || !component->m_enabled || component->m_removeRequested)
			continue;

		EnsureComponentStarted(*component);

		if (component->m_removeRequested)
			continue;

		component->OnFixedUpdate();

		// ! stop immediately if destruction is requested
		if (m_destroyRequested)
			break;
	}
}

void GameObject::Update() {
	if (!m_active || m_destroyRequested)
		return;

	Initialize();

	for (const auto& component : m_components) {
		if (!component || !component->m_enabled || component->m_removeRequested)
			continue;

		EnsureComponentStarted(*component);

		if (component->m_removeRequested)
			continue;

		component->OnUpdate();

		// ! stop immediately if destruction is requested
		if (m_destroyRequested)
			break;
	}
}

void GameObject::LateUpdate() {
	if (!m_active || m_destroyRequested)
		return;

	for (const auto& component : m_components) {
		if (!component || !component->m_enabled || component->m_removeRequested)
			continue;

		component->OnLateUpdate();

		// ! stop immediately if destruction is requested
		if (m_destroyRequested)
			break;
	}
}

void GameObject::Render() {
	if (!m_active || m_destroyRequested)
		return;

	for (const auto& component : m_components) {
		if (!component || !component->m_enabled || component->m_removeRequested)
			continue;

		component->OnRender();
	}
}

void GameObject::EndFrame() {
	RemoveRequestedComponents();
}

void GameObject::DestroyComponents() noexcept {
	if (m_componentsDestroyed) return;

	// destroy in reverse order
	// as later components might be dependent on components added before them
	for (
		auto iterator = m_components.rbegin();
		iterator != m_components.rend();
		iterator++
		) {
		if (!(*iterator) || (*iterator)->m_destroyed)
			continue;

		(*iterator)->OnDestroy();
		(*iterator)->m_destroyed = true;
	}

	m_components.clear();

	m_componentsDestroyed = true;
}