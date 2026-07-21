#include "Player.h"

#include <DirectXColors.h>

#include <algorithm>
#include <stdexcept>

#include <SNX/Core/TextureFactory.h>

void Player::Initialize(ID3D11Device* device) {
	if (!device)
		throw std::invalid_argument("Player::Initialize received a null device.");

	CreateTexture(device);
}

void Player::Update(const DirectX::Keyboard::State& keyboardState, float deltaTime, int screenWidth, int screenHeight) {
	// invincibility timer
	if (m_invincibleTimer > 0.0f)
		m_invincibleTimer -= deltaTime;

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

	// flicker during invincibility
	if (m_invincibleTimer > 0.0f) {
		const int flickerFrame = static_cast<int>(m_invincibleTimer * 20.0f);
		if (flickerFrame % 2 == 0)
			return;
	}

	RECT destinationRec = {};

	// forcing the texture to fit inside the rect with the size as we want
	destinationRec.left = static_cast<LONG>(m_pos.x);
	destinationRec.top = static_cast<LONG>(m_pos.y);
	destinationRec.right = static_cast<LONG>(m_pos.x + Size);
	destinationRec.bottom = static_cast<LONG>(m_pos.y + Size);

	// draw player
	spriteBatch->Draw(
		m_texture.Get(),
		destinationRec,
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

void Player::Reset(int screenWidth, int screenHeight) noexcept {
	// reset position when game restarts
	// bottom center
	m_pos = DirectX::SimpleMath::Vector2(
		screenWidth * .5f - Size * .5f,
		screenHeight - Size - 40.0f
	);

	m_invincibleTimer = 0.0f;
}

RectF Player::GetBounds() const noexcept {
	return RectF{
		m_pos.x,
		m_pos.y,
		static_cast<float>(Size),
		static_cast<float>(Size),
	};
}

bool Player::IsInvincible() const noexcept {
	return m_invincibleTimer > 0.0f;
}

void Player::StartInvincibility() noexcept {
	m_invincibleTimer = InvincibleDuration;
}

// create a simple blue square with white border
void Player::CreateTexture(ID3D11Device* device) {
	// actual image texture
	try {
		m_texture = TextureFactory::LoadTextureFromFile(
			device,
			L"Assets/Textures/player.png"
		);
	}
	// fallback debug texture
	catch (...) {
		m_texture = TextureFactory::CreateRectangleTexture(
			device,
			Size,
			Size,
			TextureFactory::ColorRGBA{ 80, 180, 255, 255 },
			TextureFactory::ColorRGBA{ 255, 255, 255, 255 },
			4
		);
	}
}