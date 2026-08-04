#include "Scene.h"
#include "SceneManager.h"

Scene::Scene(SceneManager& sceneManager, SceneContext& context) noexcept :
	m_sceneManager(sceneManager),
	m_context(context) {}

void Scene::Load() {
	if (m_loaded) return;

	m_loaded = true;
	OnLoad();
}

void Scene::Unload() {
	if (!m_loaded) return;

	/*
	* allow the derived scene to release its
	* scene-specific non-gameobject resources first
	*/
	OnUnload();

	// all objects belonging to this scene disappear
	m_gameObjects.Clear();
	m_loaded = false;
}

void Scene::BeginFrame() {
	if (!m_loaded) return;

	m_gameObjects.BeginFrame();
}

void Scene::FixedUpdate() {
	if (!m_loaded) return;

	OnFixedUpdate();

	if (ShouldUpdateGameObjects())
		m_gameObjects.FixedUpdate();
}

void Scene::Update() {
	if (!m_loaded) return;

	OnUpdate();

	if (ShouldUpdateGameObjects())
		m_gameObjects.Update();
}

void Scene::LateUpdate() {
	if (!m_loaded) return;

	OnLateUpdate();

	if (ShouldUpdateGameObjects())
		m_gameObjects.LateUpdate();
}

void Scene::RenderWorld() {
	if (!m_loaded) return;

	OnRenderWorld();
}

void Scene::RenderUI() {
	if (!m_loaded) return;

	OnRenderUI();
}

void Scene::EndFrame() {
	if (!m_loaded) return;

	m_gameObjects.EndFrame();
}

bool Scene::RequestSceneLoad(SceneId sceneId) {
	return m_sceneManager.LoadScene(sceneId);
}

bool Scene::RequestSceneLoad(std::string_view sceneName) {
	return m_sceneManager.LoadScene(sceneName);
}

void Scene::RequestQuit() const {
	m_context.RequestQuit();
}