#pragma once

struct RectF {
	float x;
	float y;
	float width;
	float height;
};

inline bool Intersects(const RectF& a, const RectF& b) noexcept {
	return
		a.x < b.x + b.width &&
		a.x + a.width > b.x &&
		a.y < b.y + b.height &&
		a.y + a.height > b.y;
}