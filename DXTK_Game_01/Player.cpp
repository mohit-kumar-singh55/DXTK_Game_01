#include "Player.h"

#include <DirectXColors.h>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

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
	struct Pixel {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	constexpr int width = Size;
	constexpr int height = Size;

	std::vector<Pixel> pixels(width * height);

	// set the color
	for (int y = 0;y < height;y++) {
		for (int x = 0;x < width;x++) {
			// making the 4px border around player
			bool isBorder =
				x < 4 ||
				x >= width - 4 ||
				y < 4 ||
				y >= height - 4;

			Pixel color{};

			if (isBorder)
				color = Pixel{ 255, 255, 255, 255 };	// white border
			else
				color = Pixel{ 80, 180, 255, 255 };		// blue inside

			pixels[y * width + x] = color;
		}
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = pixels.data();
	initData.SysMemPitch = width * sizeof(Pixel);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

	// create 2d texture with the above data
	HRESULT res = device->CreateTexture2D(
		&textureDesc,
		&initData,
		&texture
	);

	if (FAILED(res))
		throw std::runtime_error("Failed to create the player texture.");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// create shader resorce view (sprite batch uses shader resource view, not ID3D11Texture2D)
	res = device->CreateShaderResourceView(
		texture.Get(),
		&srvDesc,
		&m_texture
	);

	if (FAILED(res))
		throw std::runtime_error("Failed to create the player shader resource view.");
}