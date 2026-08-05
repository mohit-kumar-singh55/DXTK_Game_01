#include "TitleScene.h"

#include "SceneIds.h"

#include <SNX/Graphics/DeviceResources.h>
#include <SNX/Input/InputManager.h>

#include <DirectXColors.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>

TitleScene::TitleScene(SceneManager& sceneManager, SceneContext& context) noexcept :
	Scene(sceneManager, context) {}

void TitleScene::OnLoad() {
	auto& input = InputManager::Get();
	input.SetMouseMode(DirectX::Mouse::MODE_ABSOLUTE);
	input.Reset();
}

void TitleScene::OnUpdate() {
	auto& input = InputManager::Get();

	if (input.IsKeyPressed(DirectX::Keyboard::Escape)) {
		RequestQuit();
		return;
	}

	if (input.IsKeyPressed(DirectX::Keyboard::D1)) {
		RequestSceneLoad(SceneIds::Shooter);
		return;
	}

	if (input.IsKeyPressed(DirectX::Keyboard::D2)) {
		RequestSceneLoad(SceneIds::Tank);
		return;
	}
}

void TitleScene::OnRenderUI() {
	using DirectX::SimpleMath::Vector2;

	auto& context = GetContext();

	auto& spriteBatch = context.spriteBatch;
	auto& font = context.font;

	const float windowWidth = static_cast<float>(context.deviceResources.GetWidth());
	const float windowHeight = static_cast<float>(context.deviceResources.GetHeight());

	constexpr const wchar_t* titleText = L"PROTOTYPE ALPHA";
	constexpr const wchar_t* shooterText = L"Press 1: 2D Shooter";
	constexpr const wchar_t* tankText = L"Press 2: 3D Arena";
	constexpr const wchar_t* quitText = L"Esc : Quit";

	const Vector2 titleSize = font.MeasureString(titleText);
	const Vector2 shooterSize = font.MeasureString(shooterText);
	const Vector2 tankSize = font.MeasureString(tankText);
	const Vector2 quitSize = font.MeasureString(quitText);

	font.DrawString(
		&spriteBatch,
		titleText,
		Vector2(
			windowWidth * 0.5f -
			titleSize.x * 0.5f,
			windowHeight * 0.5f -
			120.0f
		),
		DirectX::Colors::DarkBlue
	);

	font.DrawString(
		&spriteBatch,
		shooterText,
		Vector2(
			windowWidth * 0.5f -
			shooterSize.x * 0.5f,
			windowHeight * 0.5f -
			40.0f
		),
		DirectX::Colors::DarkMagenta
	);

	font.DrawString(
		&spriteBatch,
		tankText,
		Vector2(
			windowWidth * 0.5f -
			tankSize.x * 0.5f,
			windowHeight * 0.5f +
			10.0f
		),
		DirectX::Colors::DarkMagenta
	);

	font.DrawString(
		&spriteBatch,
		quitText,
		Vector2(
			windowWidth * 0.5f -
			quitSize.x * 0.5f,
			windowHeight * 0.5f +
			80.0f
		),
		DirectX::Colors::DarkRed
	);
}

std::array<float, 4> TitleScene::OnGetClearColor() const noexcept {
	return { 0.08f, 0.08f, 0.10f, 1.0f };
}
