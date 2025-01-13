#pragma once

#include "FCollider.h"
#include "FBoundingBox.h"

class FAABBCollider : public FCollider
{
private:
	FBoundingBox bounds;

public:
	using FCollider::FCollider;

	inline bool checkCollision(FCollider* other) override { other->checkCollisionBox(this); }
	bool checkCollisionSphere(FSphereCollider* other) override;
	bool checkCollisionBox(FAABBCollider* other) override;

	FBoundingBox getBounds() { return bounds; }
	void setBounds(FBoundingBox b) { bounds = b; }
};