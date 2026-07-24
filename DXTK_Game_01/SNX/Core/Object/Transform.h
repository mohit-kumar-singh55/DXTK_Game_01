#pragma once

#include <SimpleMath.h>

class Transform final {
public:
	// TODO: For now, local and world transformations are identical. Parent-child transforms will be added later
	DirectX::SimpleMath::Matrix GetLocalMatrix() const noexcept {
		using DirectX::SimpleMath::Matrix;

		return
			Matrix::CreateScale(scale) *
			Matrix::CreateFromQuaternion(rotation) *
			Matrix::CreateTranslation(position);
	}

public:
	DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero;
	DirectX::SimpleMath::Quaternion rotation = DirectX::SimpleMath::Quaternion::Identity;
	DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One;
};