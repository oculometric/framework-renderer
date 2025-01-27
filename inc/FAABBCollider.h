#pragma once

#include "FCollider.h"
#include "FBoundingBox.h"

class FAABBCollider : public FCollider
{
private:
	FBoundingBox bounds;

public:
	using FCollider::FCollider;

	inline int checkCollision(FCollider* other) override { return other->checkCollisionBox(this); }
	int checkCollisionSphere(FSphereCollider* other) override;
	int checkCollisionBox(FAABBCollider* other) override;

	FBoundingBox getBounds() { return bounds; }
	void setBounds(FBoundingBox b) { bounds = b; }
};