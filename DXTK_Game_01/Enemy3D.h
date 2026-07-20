#pragma once

#include <GeometricPrimitive.h>
#include <SimpleMath.h>
#include <Effects.h>

#include <d3d11.h>
#include <memory>

#include <Core/Collision.h>

enum class Enemy3DType { Normal, Fast, Heavy };

class Enemy3D {
public:
	explicit Enemy3D(
		const DirectX::SimpleMath::Vector3& position,
		Enemy3DType type = Enemy3DType::Normal
	) noexcept;

	void Initialize(ID3D11DeviceContext* context);

	void Update(
		const DirectX::SimpleMath::Vector3& playerPosition,
		float deltaTime
	);

	void Draw(
		DirectX::BasicEffect* effect,
		ID3D11InputLayout* inputLayout,
		const DirectX::SimpleMath::Matrix& view,
		const DirectX::SimpleMath::Matrix& projection
	) const;

	[[nodiscard]]
	DirectX::SimpleMath::Vector3 GetPosition() const noexcept;

	[[nodiscard]]
	bool IsActive() const noexcept;

	void Destroy() noexcept;

	[[nodiscard]]
	SphereBounds GetBounds() const noexcept;

	[[nodiscard]]
	int GetScoreValue() const noexcept;

	void TakeDamage(int damage) noexcept;

	[[nodiscard]]
	bool IsDead() const noexcept;

	[[nodiscard]]
	int GetHP() const noexcept;

	[[nodiscard]]
	int GetMaxHP() const noexcept;

	[[nodiscard]]
	float GetHealthRatio() const noexcept;

	[[nodiscard]]
	bool ShouldShowHealthBar() const noexcept;

private:
	void ApplyTypeSettings() noexcept;

private:
	std::unique_ptr<DirectX::GeometricPrimitive> m_primitive;

	DirectX::SimpleMath::Vector3 m_position;

	Enemy3DType m_type = Enemy3DType::Normal;

	bool m_isActive = true;

	float m_moveSpeed = 2.0f;
	float m_collisionRadius = 0.75f;
	float m_visualScale = m_collisionRadius * 2.0f;

	int m_scorevalue = 100;

	int m_hp = 1;
	int m_maxHp = 1;

	float m_hitFlashTimer = 0.0f;

	DirectX::SimpleMath::Vector3 m_diffuseColor = DirectX::SimpleMath::Vector3(1.0f, 0.2f, 0.15f);
	DirectX::SimpleMath::Vector3 m_emissiveColor = DirectX::SimpleMath::Vector3(0.08f, 0.01f, 0.01f);

	static constexpr float HitFlashDuration = 0.12f;
};