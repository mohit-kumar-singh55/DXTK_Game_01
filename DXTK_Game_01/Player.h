#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include <keyboard.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

#include "Collision.h"

class Player {
public:
	void Initialize(ID3D11Device* device);

	void Update(
		const DirectX::Keyboard::State& keyboardState,
		float deltaTime,
		int screenWidth,
		int screenHeight
	);

	void Draw(DirectX::SpriteBatch* spriteBatch) const;

	[[nodiscard]]
	DirectX::SimpleMath::Vector2 GetBulletSpawnPosition() const noexcept;

	void Reset() noexcept;

	[[nodiscard]]
	RectF GetBounds() const noexcept;

private:
	void CreateTexture(ID3D11Device* device);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	DirectX::SimpleMath::Vector2 m_pos{ 100.0,100.0f };

	static constexpr float Speed = 300.0f;
	static constexpr int Size = 64;
};