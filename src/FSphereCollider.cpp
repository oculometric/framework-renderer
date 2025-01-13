#include "FSphereCollider.h"

#include "FPhysicsComponent.h"
#include "FAABBCollider.h"

bool FSphereCollider::checkCollisionSphere(FSphereCollider* other)
{
	FVector global_origin_a = getOwner()->getOwner()->transform.getPosition() + getCenter();
	FVector global_origin_b = other->getOwner()->getOwner()->transform.getPosition() + other->getCenter();

	float distance = magnitude(global_origin_a - global_origin_b);
	if (distance <= getRadius() + other->getRadius())
	{
		return true;
	}

	return false;
}

bool FSphereCollider::checkCollisionBox(FAABBCollider* other)
{
	FVector oa = getOwner()->getOwner()->transform.getPosition();
	FVector ob = other->getOwner()->getOwner()->transform.getPosition();

	FBoundingBox bb = other->getBounds();
	bb.max_corner = bb.max_corner + ob;
	bb.min_corner = bb.min_corner + ob;

	return false; // TODO: implement box-sphere collision detection
}
