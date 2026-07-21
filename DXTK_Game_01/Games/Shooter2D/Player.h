#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include <keyboard.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>

#include <SNX/Core/Collision.h>

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

	void Reset(int screenWidth, int screenHeight) noexcept;

	[[nodiscard]]
	RectF GetBounds() const noexcept;

	[[nodiscard]]
	bool IsInvincible() const noexcept;

	void StartInvincibility() noexcept;

private:
	void CreateTexture(ID3D11Device* device);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	DirectX::SimpleMath::Vector2 m_pos{ 100.0,100.0f };

	float m_invincibleTimer = 0.0f;

	static constexpr float Speed = 300.0f;
	static constexpr int Size = 64;
	static constexpr float InvincibleDuration = 1.5f;
};