#include "FAABBCollider.h"

#include "FSphereCollider.h"
#include "FPhysicsComponent.h"

bool FAABBCollider::checkCollisionSphere(FSphereCollider* other)
{
	return other->checkCollisionBox(this);
}

bool FAABBCollider::checkCollisionBox(FAABBCollider* other)
{
	FVector oa = getOwner()->getOwner()->transform.getPosition();
	FVector ob = other->getOwner()->getOwner()->transform.getPosition();

	// TODO: should we be fully recomputing BBs each time? probably
	FBoundingBox ba = getBounds();
	ba.max_corner = ba.max_corner + oa;
	ba.min_corner = ba.min_corner + oa;
	FBoundingBox bb = other->getBounds();
	bb.max_corner = bb.max_corner + ob;
	bb.min_corner = bb.min_corner + ob;

	if (ba.max_corner.x < bb.min_corner.x || bb.max_corner.x < ba.min_corner.x)
		return false;
	if (ba.max_corner.y < bb.min_corner.y || bb.max_corner.y < ba.min_corner.y)
		return false;
	if (ba.max_corner.z < bb.min_corner.z || bb.max_corner.z < ba.min_corner.z)
		return false;

	return true;
}
