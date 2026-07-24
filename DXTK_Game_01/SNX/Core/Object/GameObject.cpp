#include "GameObject.h"

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
	if (!component.m_enabled || component.m_started)
		return;

	component.OnStart();
	component.m_started = true;
}

void GameObject::FixedUpdate() {
	if (!m_active || m_destroyRequested)
		return;

	Initialize();

	for (const auto& component : m_components) {
		if (!component->m_enabled)
			continue;

		EnsureComponentStarted(*component);
		component->OnFixedUpdate();
	}
}

void GameObject::Update() {
	if (!m_active || m_destroyRequested)
		return;

	Initialize();

	for (const auto& component : m_components) {
		if (!component->m_enabled)
			continue;

		EnsureComponentStarted(*component);
		component->OnUpdate();
	}
}

void GameObject::LateUpdate() {
	if (!m_active || m_destroyRequested)
		return;

	for (const auto& component : m_components) {
		if (!component->m_enabled)
			continue;

		component->OnLateUpdate();
	}
}

void GameObject::Render() {
	if (!m_active || m_destroyRequested)
		return;

	for (const auto& component : m_components) {
		if (!component->m_enabled)
			continue;

		component->OnRender();
	}
}

void GameObject::DestroyComponents() noexcept {
	if (m_componentsDestroyed) return;

	// destroy in reverse order
	// as later components might be dependent on components added before them
	for (
		auto iterator = m_components.rbegin();
		iterator != m_components.rend();
		iterator++
		)
		(*iterator)->OnDestroy();

	m_components.clear();

	m_componentsDestroyed = true;
}