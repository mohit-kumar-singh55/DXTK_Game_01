#include "TankVisual.h"

bool TankVisual::Load(
	ID3D11Device* device,
	DirectX::IEffectFactory& effectFactory
) noexcept {
	m_bodyLoaded = m_body.LoadFromSDKMESH(
		device,
		effectFactory,
		L"Assets\\Models\\Runtime\\Tank\\tank_body.sdkmesh"
	);

	m_turretLoaded = m_turret.LoadFromSDKMESH(
		device,
		effectFactory,
		L"Assets\\Models\\Runtime\\Tank\\tank_turret.sdkmesh"
	);

	// ! texture dir is being set from effect factory from Game.cpp

	return IsLoaded();
}

void TankVisual::UpdateEffects(
	const DirectX::SimpleMath::Vector3& fogColor,
	float fogStart,
	float fogEnd
) {
	if (m_bodyLoaded)
		m_body.UpdateEffects(fogColor, fogStart, fogEnd);

	if (m_turretLoaded)
		m_turret.UpdateEffects(fogColor, fogStart, fogEnd);
}

void TankVisual::SetWorldTransform(
	const DirectX::SimpleMath::Vector3& position,
	float bodyYaw,
	float turretYaw
) noexcept {
	m_position = position;
	m_bodyYaw = bodyYaw;
	m_turretYaw = turretYaw;
}

void TankVisual::Draw(
	ID3D11DeviceContext* context,
	DirectX::CommonStates& states,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& projection
) const {
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Matrix;

	const Matrix bodyWorld =
		Matrix::CreateScale(m_scale) *
		Matrix::CreateRotationY(m_bodyYaw) *
		Matrix::CreateTranslation(m_position);

	const Matrix turretWorld =
		Matrix::CreateScale(m_scale) *
		Matrix::CreateRotationY(m_turretYaw) *
		Matrix::CreateTranslation(m_position);

	if (m_bodyLoaded)
		m_body.DrawWithTransform(context, states, bodyWorld, view, projection);

	if (m_turretLoaded)
		m_turret.DrawWithTransform(context, states, turretWorld, view, projection);
}

bool TankVisual::IsLoaded() const noexcept {
	return m_bodyLoaded || m_turretLoaded;
}
