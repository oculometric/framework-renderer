#pragma once

#include "FCollider.h"
#include "FVector.h"

class FSphereCollider : public FCollider
{
private:
	float radius = 1.0f;
	FVector center = FVector(0, 0, 0);

public:
	using FCollider::FCollider;

	inline int checkCollision(FCollider* other) override { return other->checkCollisionSphere(this); }
	int checkCollisionSphere(FSphereCollider* other) override;
	int checkCollisionBox(FAABBCollider* other) override;

	float getRadius() { return radius; }
	void setRadius(float r) { radius = r; }

	FVector getCenter() { return center; }
	void setCenter(FVector v) { center = v; }
};