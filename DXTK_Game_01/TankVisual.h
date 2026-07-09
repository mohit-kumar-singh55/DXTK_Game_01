#pragma once

#include "ModelObject3D.h"

#include <CommonStates.h>
#include <Effects.h>
#include <SimpleMath.h>

#include <d3d11.h>

class TankVisual {
public:
	bool Load(
		ID3D11Device* device,
		DirectX::IEffectFactory& effectFactory
	) noexcept;

	void UpdateEffects(
		const DirectX::SimpleMath::Vector3& fogColor,
		float fogStart,
		float fogEnd
	);

	void SetWorldTransform(
		const DirectX::SimpleMath::Vector3& position,
		float bodyYaw,
		float turretYaw
	) noexcept;

	void Draw(
		ID3D11DeviceContext* context,
		DirectX::CommonStates& states,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	[[nodiscard]]
	bool IsLoaded() const noexcept;

private:
	ModelObject3D m_body;
	ModelObject3D m_turret;

	bool m_bodyLoaded = false;
	bool m_turretLoaded = false;

	DirectX::SimpleMath::Vector3 m_position = DirectX::SimpleMath::Vector3::Zero;

	float m_bodyYaw = 0.0f;
	float m_turretYaw = 0.0f;

	float m_scale = 0.4f;
};