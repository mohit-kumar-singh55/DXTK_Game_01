#pragma once

#include <functional>
#include <utility>

class DeviceResources;
class AudioManager;

namespace DirectX {
	class SpriteBatch;
	class SpriteFont;
}

struct SceneContext final {
	SceneContext(
		DeviceResources& deviceResourcesValue,
		DirectX::SpriteBatch& spriteBatchValue,
		DirectX::SpriteFont& fontValue,
		AudioManager& audioManagerValue,
		std::function<void()> requestQuitCallback
	) :
		deviceResources(deviceResourcesValue),
		spriteBatch(spriteBatchValue),
		font(fontValue),
		audioManager(audioManagerValue),
		requestQuit(std::move(requestQuitCallback)) {}

	void RequestQuit() const {
		if (requestQuit)
			requestQuit();
	}

	DeviceResources& deviceResources;
	DirectX::SpriteBatch& spriteBatch;
	DirectX::SpriteFont& font;
	AudioManager& audioManager;

private:
	std::function<void()> requestQuit;
};