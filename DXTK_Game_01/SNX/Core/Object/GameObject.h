#pragma once

#include <SNX/Core/Object/Component.h>
#include <SNX/Core/Object/Transform.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class GameObject final {
public:
	explicit GameObject(std::string name = "GameObject");
	~GameObject();

	// disallow to copy or move
	GameObject(const GameObject&) = delete;
	GameObject& operator=(const GameObject&) = delete;
	GameObject(GameObject&&) = delete;
	GameObject& operator=(GameObject&&) = delete;

	template<typename T, typename... Arguments>
	T& AddComponent(Arguments&&... arguments) {
		static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");

		auto component = std::make_unique<T>(*this, std::forward<Arguments>(arguments)...);

		T& componentReference = *component;

		m_components.push_back(std::move(component));

		if (m_initialized) {
			componentReference.OnInitialize();
			componentReference.m_initialized = true;
		}

		return componentReference;
	}

	template<typename T>
	[[nodiscard]]
	T* GetComponent() noexcept {
		static_assert(std::is_base_of_v<Component, T>, "T mush inherit from Component");

		for (const auto& component : m_components)
			if (T* result = dynamic_cast<T*>(component.get()))
				return result;

		return nullptr;
	}

	void Initialize();

	void FixedUpdate();
	void Update();
	void LateUpdate();
	void Render();

	void RequestDestroy() noexcept { m_destroyRequested = true; }

	[[nodiscard]]
	bool IsDestroyRequested() const noexcept { return m_destroyRequested; }

	[[nodiscard]]
	bool IsActive() const noexcept { return m_active; }

	void SetActive(bool active) noexcept { m_active = active; }

	[[nodiscard]]
	const std::string& GetName() const noexcept { return m_name; }

	Transform& GetTransform()  noexcept { return m_transform; }

	const Transform& GetTransform() const noexcept { return m_transform; }

private:
	void EnsureComponentStarted(Component& component);

	void DestroyComponents() noexcept;

private:
	std::string m_name;

	Transform m_transform;

	std::vector<std::unique_ptr<Component>> m_components;

	bool m_initialized = false;
	bool m_active = true;
	bool m_destroyRequested = false;
	bool m_componentsDestroyed = false;
};