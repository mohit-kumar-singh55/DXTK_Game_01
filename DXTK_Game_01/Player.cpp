#include "Player.h"

#include <DirectXColors.h>

#include <algorithm>
#include <stdexcept>

#include "TextureFactory.h"

void Player::Initialize(ID3D11Device* device) {
	if (!device)
		throw std::invalid_argument("Player::Initialize received a null device.");

	CreateTexture(device);
}

void Player::Update(const DirectX::Keyboard::State& keyboardState, float deltaTime, int screenWidth, int screenHeight) {
	using DirectX::SimpleMath::Vector2;

	// move player
	Vector2 move = DirectX::SimpleMath::Vector2::Zero;

	// take input
	if (keyboardState.W || keyboardState.Up)
		move.y -= 1.0f;
	if (keyboardState.S || keyboardState.Down)
		move.y += 1.0f;
	if (keyboardState.A || keyboardState.Left)
		move.x -= 1.0f;
	if (keyboardState.D || keyboardState.Right)
		move.x += 1.0f;

	if (move.LengthSquared() > 0.0f) {
		move.Normalize();
		m_pos += move * Speed * deltaTime;
	}

	// clamp inside window
	m_pos.x = std::clamp(
		m_pos.x,
		0.0f,
		static_cast<float>(screenWidth - Size)
	);

	m_pos.y = std::clamp(
		m_pos.y,
		0.0f,
		static_cast<float>(screenHeight - Size)
	);
}

void Player::Draw(DirectX::SpriteBatch* spriteBatch) const {
	if (!spriteBatch || !m_texture)
		return;

	// draw player
	spriteBatch->Draw(
		m_texture.Get(),
		m_pos,
		DirectX::Colors::White
	);
}

DirectX::SimpleMath::Vector2 Player::GetBulletSpawnPosition() const noexcept {
	// returning top-center pos of the player sprite
	return DirectX::SimpleMath::Vector2(
		m_pos.x + static_cast<float>(Size) * 0.5f,
		m_pos.y
	);
}

void Player::Reset() noexcept {
	// reset position when game restarts
	m_pos = DirectX::SimpleMath::Vector2(100.0f, 100.0f);
}

// create a simple blue square with white border
void Player::CreateTexture(ID3D11Device* device) {
	m_texture = TextureFactory::CreateRectangleTexture(
		device,
		Size,
		Size,
		TextureFactory::ColorRGBA{ 80, 180, 255, 255 },
		TextureFactory::ColorRGBA{ 255, 255, 255, 255 },
		4
	);
}