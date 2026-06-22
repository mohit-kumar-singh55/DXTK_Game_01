#define WIN32_LEAN_AND_MEAN

// window related
#include <windows.h>
#include <wrl/client.h>

#include <d3d11.h>
#include <dxgi.h>

#include <memory>
#include <stdexcept>

// game related
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <random>
#include <string>

// DirectXTK
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <Keyboard.h>
#include <SimpleMath.h>
#include <DirectXColors.h>

#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

// -----------------------------------------------------------------------------
// Global variables for now.
// Later we will move these into Game / DeviceResources classes.
// -----------------------------------------------------------------------------

HWND g_window = nullptr;								// window handle, needs a window before we can render it

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

ComPtr<ID3D11Device> g_device;							// device creates GPU resources			(context sends commands to the GPU)
ComPtr<ID3D11DeviceContext> g_context;					// context sends commands to the GPU		(Context = GPU command sender)
ComPtr<IDXGISwapChain> g_swapChain;						// swap chain owns the screen buffers	(Swap chain = screen buffer manager)
ComPtr<ID3D11RenderTargetView> g_renderTargetView;		// This is the image we are drawing into	(In Unity terms, it is similar to the camera’s render target / back buffer)

// game related
using Clock = std::chrono::steady_clock;
Clock::time_point g_previousTime;						// used for delta time

// DirectXTK objects
std::unique_ptr<DirectX::SpriteBatch> g_spriteBatch;
std::unique_ptr<DirectX::Keyboard> g_keyboard;

std::unique_ptr<Player> g_player;

std::vector<Bullet> g_bullets;
ComPtr<ID3D11ShaderResourceView> g_bulletTexture;
DirectX::Keyboard::KeyboardStateTracker g_keyboardTracker;

std::vector<Enemy> g_enemies;
ComPtr<ID3D11ShaderResourceView> g_enemyTexture;
float g_enemySpawnTimer = 0.0f;
constexpr float ENEMY_SPAWN_INTERVAL = 1.0f;
std::mt19937 g_randomEngine{ std::random_device{}() };

std::unique_ptr<DirectX::SpriteFont> g_font;	// ui text renderer

int g_score = 0;

constexpr int PLAYER_MAX_HP = 3;
int g_playerHp = PLAYER_MAX_HP;

enum class GameState {
	Title,
	Playing,
	GameOver
};

GameState g_gameState = GameState::Title;

// -----------------------------------------------------------------------------
// Helper
// -----------------------------------------------------------------------------

void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		throw std::runtime_error("HRESULT failed.");
	}
}

// -----------------------------------------------------------------------------
// Window Procedure
// -----------------------------------------------------------------------------

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// DirectXTK Keyboard needs this to receive keyboard messages.
	DirectX::Keyboard::ProcessMessage(message, wParam, lParam);

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

// -----------------------------------------------------------------------------
// Create Win32 window
// -----------------------------------------------------------------------------

void CreateGameWindow(HINSTANCE instance, int showCommand) {
	const wchar_t CLASS_NAME[] = L"DXTKGameWindowClass";

	// create window class instance
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);

	// define window styles
	DWORD windowStyle =
		WS_OVERLAPPED |
		WS_CAPTION |
		WS_SYSMENU |
		WS_MINIMIZEBOX;

	// define window rect
	RECT rect = {};
	rect.left = 0;
	rect.top = 0;
	rect.right = WINDOW_WIDTH;
	rect.bottom = WINDOW_HEIGHT;

	// resize the window
	AdjustWindowRect(&rect, windowStyle, FALSE);

	// calc window size
	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	// create the window based on above settings
	g_window = CreateWindowEx(
		0,
		CLASS_NAME,
		L"DirectXTK Game 01",
		windowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		instance,
		nullptr
	);

	// throw error if unable to create window
	if (!g_window)
		throw std::runtime_error("Failed to create window.");

	// finally, draw the window
	ShowWindow(g_window, showCommand);
}

// create one shared bullet texture
void CreateBulletTuxture() {
	struct Pixel {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	constexpr int width = Bullet::TextureWidth;
	constexpr int height = Bullet::TextureHeight;

	std::vector<Pixel> pixels(width * height);

	// set the color
	for (int y = 0;y < height;y++) {
		for (int x = 0;x < width;x++) {
			// making the 4px border around player
			bool isBorder =
				x == 0 ||
				x == width - 1 ||
				y == 0 ||
				y == height - 1;

			Pixel color{};

			if (isBorder)
				color = Pixel{ 255, 255, 255, 255 };	// white border
			else
				color = Pixel{ 255, 210, 50, 255 };		// golden yellow inside

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

	ComPtr<ID3D11Texture2D> texture;

	// create 2d texture with the above data
	ThrowIfFailed(
		g_device->CreateTexture2D(
			&textureDesc,
			&initData,
			&texture
		)
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// create shader resorce view (sprite batch uses shader resource view, not ID3D11Texture2D)
	ThrowIfFailed(
		g_device->CreateShaderResourceView(
			texture.Get(),
			&srvDesc,
			&g_bulletTexture
		)
	);
}

// create one shared enemy texture
void CreateEnemyTuxture() {
	struct Pixel {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};

	constexpr int width = Enemy::TextureWidth;
	constexpr int height = Enemy::TextureHeight;

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
				color = Pixel{ 255, 80, 100, 255 };		// coral pink inside

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

	ComPtr<ID3D11Texture2D> texture;

	// create 2d texture with the above data
	ThrowIfFailed(
		g_device->CreateTexture2D(
			&textureDesc,
			&initData,
			&texture
		)
	);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	// create shader resorce view (sprite batch uses shader resource view, not ID3D11Texture2D)
	ThrowIfFailed(
		g_device->CreateShaderResourceView(
			texture.Get(),
			&srvDesc,
			&g_enemyTexture
		)
	);
}

// -----------------------------------------------------------------------------
// Initialize Direct3D 11
// -----------------------------------------------------------------------------

void InitializeDirect3D()
{
	// create swap chain settings instance
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	// swap chain buffer settings
	swapChainDesc.BufferDesc.Width = WINDOW_WIDTH;
	swapChainDesc.BufferDesc.Height = WINDOW_HEIGHT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;			// ! FPS
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	// swap chain sample settings
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// swap chain settings
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = g_window;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// define feature levels
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
	};

	// define created feature level
	D3D_FEATURE_LEVEL createdFeatureLevel{};

	// create device and swap chain
	ThrowIfFailed(
		D3D11CreateDeviceAndSwapChain(
			nullptr,                    // Adapter (mean GPU) if nullptr -> DX will choose itself
			D3D_DRIVER_TYPE_HARDWARE,   // Use GPU hardware
			nullptr,                    // Software rasterizer (Rasterizer is the stage in the graphics pipeline that converts triangles into pixels (fragments))
			0,                          // Flags
			featureLevels,
			1,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&g_swapChain,
			&g_device,
			&createdFeatureLevel,
			&g_context
		)
	);

	// Get back buffer from swap chain.
	ComPtr<ID3D11Texture2D> backBuffer;

	ThrowIfFailed(
		g_swapChain->GetBuffer(
			0,
			IID_PPV_ARGS(&backBuffer)
		)
	);

	// Create render target view.
	ThrowIfFailed(
		g_device->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&g_renderTargetView
		)
	);

	// TODO:  from here create DirectXTK objects.
	g_spriteBatch = std::make_unique<DirectX::SpriteBatch>(g_context.Get());
	g_keyboard = std::make_unique<DirectX::Keyboard>();
	g_font = std::make_unique<DirectX::SpriteFont>(g_device.Get(), L"Assets/gamefont.spritefont");

	// create player and its texture
	g_player = std::make_unique<Player>();
	g_player->Initialize(g_device.Get());

	// create the shared bullet texture
	CreateBulletTuxture();
	// create the shared enemy texture
	CreateEnemyTuxture();
}

// spawn enemies
void SpawnEnemy() {
	// Generate random float values between 0.0f and WINDOW_WIDTH - Enemy::TextureWidth
	std::uniform_real_distribution<float> distribution(
		0.0f,
		static_cast<float>(WINDOW_WIDTH - Enemy::TextureWidth)
	);

	const float x = distribution(g_randomEngine);

	DirectX::SimpleMath::Vector2 spawnPos{
		x,
		-static_cast<float>(Enemy::TextureHeight)
	};

	g_enemies.emplace_back(spawnPos);
}

// start game
void StartGame() {
	g_score = 0;
	g_playerHp = PLAYER_MAX_HP;

	g_bullets.clear();
	g_enemies.clear();

	g_enemySpawnTimer = 0.0f;

	if (g_player)
		g_player->Reset();

	g_gameState = GameState::Playing;
}

// -----------------------------------------------------------------------------
// Update
// -----------------------------------------------------------------------------

void Update(float deltaTime)
{
	// use keyboardState for continuous actions (will be true for all frame if the key is held)
	// use keyboardTracker for per frame actions (will be true only for the frame when the key is pressed)
	const auto keyboardState = g_keyboard->GetState();

	// compare the current keyboard state with the previous frame
	g_keyboardTracker.Update(keyboardState);

	// close window
	if (keyboardState.Escape)
		PostQuitMessage(0);

	switch (g_gameState) {
	case GameState::Title: {
		if (g_keyboardTracker.pressed.Enter)
			StartGame();
		break;
	}

						 // update things
	case GameState::Playing: {
		// update objects
		g_player->Update(keyboardState, deltaTime, WINDOW_WIDTH, WINDOW_HEIGHT);

		// spawn bullet only once when space changes from released to pressed
		if (g_keyboardTracker.pressed.Space)
			// emplace_back constructs the object and push at the back
			g_bullets.emplace_back(g_player->GetBulletSpawnPosition());

		// update bullets
		for (Bullet& bullet : g_bullets)
			bullet.Update(deltaTime);

		// spawn enemies
		g_enemySpawnTimer += deltaTime;
		if (g_enemySpawnTimer >= ENEMY_SPAWN_INTERVAL) {
			g_enemySpawnTimer -= ENEMY_SPAWN_INTERVAL;
			SpawnEnemy();
		}

		// update enemies
		for (Enemy& enemy : g_enemies)
			enemy.Update(deltaTime);

		// enemy reaching bottom damages the player
		for (Enemy& enemy : g_enemies) {
			if (!enemy.IsActive())
				continue;

			if (enemy.IsOutsideScreen(WINDOW_HEIGHT)) {
				enemy.Destroy();
				--g_playerHp;
				if (g_playerHp <= 0) {
					g_playerHp = 0;
					g_gameState = GameState::GameOver;
				}
			}
		}

		// bullet vs enemy collision
		for (Bullet& bullet : g_bullets) {
			if (!bullet.IsActive())
				continue;

			for (Enemy& enemy : g_enemies) {
				if (!enemy.IsActive())
					continue;

				if (Intersects(bullet.GetBounds(), enemy.GetBounds())) {
					OutputDebugStringA("COLLIDED!!!\n");
					bullet.Destroy();
					enemy.Destroy();
					g_score += 100;
					break;
				}
			}
		}

		// or instead use erase_if for shorter version
		g_bullets.erase(
			// rearranges the vector so unwanted objects are moved to the end, then returns the beginning of that unwanted range
			// after that, erase truly removes them
			std::remove_if(
				g_bullets.begin(),
				g_bullets.end(),
				[](const Bullet& bullet) {
					return
						!bullet.IsActive() ||
						bullet.IsOutsideScreen();
				}
			),
			g_bullets.end()
		);

		// erase enemies if not active
		g_enemies.erase(
			// rearranges the vector so unwanted objects are moved to the end, then returns the beginning of that unwanted range
			// after that, erase truly removes them
			std::remove_if(
				g_enemies.begin(),
				g_enemies.end(),
				[](const Enemy& enemy) {
					return
						!enemy.IsActive() ||
						enemy.IsOutsideScreen(WINDOW_HEIGHT);
				}
			),
			g_enemies.end()
		);
		break;
	}

	case GameState::GameOver: {
		if (g_keyboardTracker.pressed.R)
			StartGame();
		break;
	}

	default:
		break;
	}
}

// drawing ui (font)
void DrawUI() {
	if (!g_font) return;

	using DirectX::SimpleMath::Vector2;

	switch (g_gameState) {
	case GameState::Title: {
		// title
		g_font->DrawString(
			g_spriteBatch.get(),
			L"DXTK SHOOTER",
			Vector2(
				WINDOW_WIDTH * 0.5f - 80.0f,
				WINDOW_HEIGHT * 0.5f - 150.0f
			),
			DirectX::Colors::White
		);

		// instructon
		g_font->DrawString(
			g_spriteBatch.get(),
			L"Press ENTER to Start",
			Vector2(
				WINDOW_WIDTH * 0.5f - 170.0f,
				WINDOW_HEIGHT * 0.5f - 20.0f
			),
			DirectX::Colors::Yellow
		);

		// controls
		g_font->DrawString(
			g_spriteBatch.get(),
			L"WASD / Arrows: Move\nSpace: Shoot\nEsc: Quit",
			Vector2(
				WINDOW_WIDTH * 0.5f - 400.0f,
				WINDOW_HEIGHT * 0.5f + 150.0f
			),
			DirectX::Colors::White
		);
		break;
	}

	case GameState::Playing: {
		const std::wstring scoreText = L"Score: " + std::to_wstring(g_score);
		const std::wstring hpText = L"HP: " + std::to_wstring(g_playerHp);

		// draw strings
		g_font->DrawString(
			g_spriteBatch.get(),
			scoreText.c_str(),
			DirectX::SimpleMath::Vector2(20.0f, 20.0f),
			DirectX::Colors::White
		);

		g_font->DrawString(
			g_spriteBatch.get(),
			hpText.c_str(),
			DirectX::SimpleMath::Vector2(20.0f, 60.0f),
			DirectX::Colors::White
		);
		break;
	}

	case GameState::GameOver: {
		const std::wstring scoreText = L"Final Score:" + std::to_wstring(g_score);

		g_font->DrawString(
			g_spriteBatch.get(),
			L"GAME OVER",
			Vector2(
				WINDOW_WIDTH * 0.5f - 80.0f,
				WINDOW_HEIGHT * 0.5f - 100.0f
			),
			DirectX::Colors::Yellow
		);

		g_font->DrawString(
			g_spriteBatch.get(),
			scoreText.c_str(),
			Vector2(
				WINDOW_WIDTH * 0.5f - 130.0f,
				WINDOW_HEIGHT * 0.5f - 40.0f
			),
			DirectX::Colors::White
		);

		g_font->DrawString(
			g_spriteBatch.get(),
			L"Press R to Restart",
			Vector2(
				WINDOW_WIDTH * 0.5f - 150.0f,
				WINDOW_HEIGHT * 0.5f + 20.0f
			),
			DirectX::Colors::White
		);
		break;
	}
	}
}

// -----------------------------------------------------------------------------
// Render
// -----------------------------------------------------------------------------

void Render()
{
	const float clearColor[4] = {
		//0.1f, 0.15f, 0.25f, 1.0f
		1.0f, 0.15f, 0.25f, 1.0f
	};

	g_context->OMSetRenderTargets(
		1,
		g_renderTargetView.GetAddressOf(),
		nullptr
	);

	// set the viewport
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(WINDOW_WIDTH);
	viewport.Height = static_cast<float>(WINDOW_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	g_context->RSSetViewports(1, &viewport);

	g_context->ClearRenderTargetView(
		g_renderTargetView.Get(),
		clearColor
	);

	// DirectXTK SpriteBatch
	g_spriteBatch->Begin();

	if (g_gameState == GameState::Playing || g_gameState == GameState::GameOver) {
		// draw enemies
		for (const Enemy& enemy : g_enemies)
			enemy.Draw(g_spriteBatch.get(), g_enemyTexture.Get());

		// drawing bullets before player, so that the player overlap the bullets
		for (const Bullet& bullet : g_bullets)
			bullet.Draw(g_spriteBatch.get(), g_bulletTexture.Get());

		// draw player
		g_player->Draw(g_spriteBatch.get());
	}

	// draw ui
	DrawUI();

	g_spriteBatch->End();

	g_swapChain->Present(1, 0);
}

// -----------------------------------------------------------------------------
// WinMain
// -----------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
	try {
		CreateGameWindow(instance, showCommand);
		InitializeDirect3D();

		g_previousTime = Clock::now();

		MSG msg = {};

		while (msg.message != WM_QUIT) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				// calculate delta time
				auto currentTime = Clock::now();

				float deltaTime = std::chrono::duration<float>(
					currentTime - g_previousTime
				).count();

				g_previousTime = currentTime;

				// ! safety clamp
				// ! this prevents a huge jump if the program freezes for a moment
				deltaTime = (deltaTime < 0.1f) ? deltaTime : 0.1f;	// min

				Update(deltaTime);
				Render();
			}
		}
	}
	catch (const std::exception& e) {
		MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
	}

	return 0;
}