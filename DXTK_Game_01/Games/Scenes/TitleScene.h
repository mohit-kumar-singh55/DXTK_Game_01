#pragma once

#include <SNX/Core/Scene/Scene.h>

class TitleScene final : public Scene {
public:
	TitleScene(SceneManager& sceneManager, SceneContext& context) noexcept;

protected:
	void OnLoad() override;
	void OnUpdate() override;
	void OnRenderUI() override;

	[[nodiscard]]
	std::array<float, 4> OnGetClearColor() const noexcept override;
};