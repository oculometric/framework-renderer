#include "FAABBCollider.h"

#include "FSphereCollider.h"
#include "FPhysicsComponent.h"
#include "FDebug.h"

inline float sign(float f) { return f > 0.0f ? 1.0f : (f < 0.0f ? -1.0f : 0.0f); }

int FAABBCollider::checkCollisionSphere(FSphereCollider* other)
{
	return other->checkCollisionBox(this);
}

int FAABBCollider::checkCollisionBox(FAABBCollider* other)
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

	bool colliding = (ba.min_corner.x <= bb.max_corner.x && ba.max_corner.x >= bb.min_corner.x)
				  && (ba.min_corner.y <= bb.max_corner.y && ba.max_corner.y >= bb.min_corner.y)
				  && (ba.min_corner.z <= bb.max_corner.z && ba.max_corner.z >= bb.min_corner.z);

	// if both are kinematic, don't bother doing a response
	if (getOwner()->kinematic && other->getOwner()->kinematic) return colliding;

	if (colliding)
	{
		// compute penetration first
		FVector center_diff = getCenter(ba) - getCenter(bb);
		FVector half_extent_sum = (getExtents(ba) + getExtents(bb)) / 2.0f;
		FVector penetration = abs(center_diff) - half_extent_sum;

		// decide which axis has the closest edge
		FVector collision_normal;
		float min_penetration = min(min(abs(penetration.x), abs(penetration.y)), abs(penetration.z));
		if (min_penetration == abs(penetration.x))
			collision_normal = FVector(sign(center_diff.x), 0, 0);
		else if (min_penetration == abs(penetration.y))
			collision_normal = FVector(0, sign(center_diff.y), 0);
		else if (min_penetration == abs(penetration.z))
			collision_normal = FVector(0, 0, sign(center_diff.z));

		FVector relative_velocity = getOwner()->getVelocity() - other->getOwner()->getVelocity();
			
		float e = other->getOwner()->restitution; if (getOwner()->restitution > e) e = getOwner()->restitution;
		float penetration_depth = -(penetration ^ collision_normal);
		float m_inv_sum = ((1.0f * getOwner()->getInvMass()) + (1.0f * other->getOwner()->getInvMass()));

		if (penetration_depth < 0.0f)
		{
			FVector reposition_vector = (collision_normal * -penetration_depth) / m_inv_sum;
			getOwner()->getOwner()->transform.translate(reposition_vector * getOwner()->getInvMass());
			other->getOwner()->getOwner()->transform.translate(reposition_vector * -other->getOwner()->getInvMass());
		}

		float vj = -(1.0f + e) * (collision_normal ^ relative_velocity);
		float j = vj / m_inv_sum;
		FVector ja = collision_normal * j * getOwner()->getInvMass();
		FVector jb = collision_normal * -j * other->getOwner()->getInvMass();
		getOwner()->applyImpulse(ja);
		other->getOwner()->applyImpulse(jb);

		bool began = getOwner()->beginColliding(other->getOwner(), collision_normal);
		other->getOwner()->beginColliding(getOwner(), -collision_normal);

		return began ? 1 : 0;
	}

	bool ended = getOwner()->endColliding(other->getOwner());
	other->getOwner()->endColliding(getOwner());

	return ended ? -1 : 0;
}
