#include "FSphereCollider.h"

#include "FPhysicsComponent.h"
#include "FAABBCollider.h"

bool FSphereCollider::checkCollisionSphere(FSphereCollider* other)
{
	FVector global_origin_a = getOwner()->getOwner()->transform.getPosition() + getCenter();
	FVector global_origin_b = other->getOwner()->getOwner()->transform.getPosition() + other->getCenter();

	float distance = magnitude(global_origin_a - global_origin_b);
	bool colliding = distance <= getRadius() + other->getRadius();

	if (getOwner()->kinematic && other->getOwner()->kinematic) return colliding;

	if (colliding)
	{
		FVector collision_normal = normalise(global_origin_a - global_origin_b);
		FVector relative_velocity = getOwner()->getVelocity() - other->getOwner()->getVelocity();
		if ((collision_normal ^ relative_velocity) < 0.0f)
		{
			float e = other->getOwner()->restitution; if (getOwner()->restitution > e) e = getOwner()->restitution;
			float penetration_depth = magnitude(global_origin_a - global_origin_b) - (getRadius() + other->getRadius());
			float m_inv_sum = ((1.0f * getOwner()->getInvMass()) + (1.0f * other->getOwner()->getInvMass()));
			if (penetration_depth < 0.0f)
			{
				FVector reposition_vector = (collision_normal * penetration_depth) / m_inv_sum;
				getOwner()->getOwner()->transform.translate(reposition_vector * getOwner()->getInvMass());
				other->getOwner()->getOwner()->transform.translate(reposition_vector * -other->getOwner()->getInvMass());
			}
			float vj = -(1.0f + e) * (collision_normal ^ relative_velocity);
			float j = vj / m_inv_sum;
			FVector ja = collision_normal * j * getOwner()->getInvMass();
			FVector jb = collision_normal * -j * other->getOwner()->getInvMass();
			getOwner()->applyImpulse(ja);
			other->getOwner()->applyImpulse(jb);
		}
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

	FVector so = oa + center;

	FVector closest_point = closestPointOnBox(bb, so);

	float distance_squared = magnitude_squared(closest_point - so);
	bool colliding = distance_squared <= radius * radius;

	if (getOwner()->kinematic && other->getOwner()->kinematic) return colliding;

	if (colliding)
	{
		FVector collision_normal = normalise(so - closest_point);
		FVector relative_velocity = getOwner()->getVelocity() - other->getOwner()->getVelocity();
		if ((collision_normal ^ relative_velocity) < 0.0f)
		{
			float e = other->getOwner()->restitution; if (getOwner()->restitution > e) e = getOwner()->restitution;
			float penetration_depth = sqrt(distance_squared) - radius;
			float m_inv_sum = ((1.0f * getOwner()->getInvMass()) + (1.0f * other->getOwner()->getInvMass()));
			if (penetration_depth < 0.0f)
			{
				FVector reposition_vector = (collision_normal * penetration_depth) / m_inv_sum;
				getOwner()->getOwner()->transform.translate(reposition_vector * getOwner()->getInvMass());
				other->getOwner()->getOwner()->transform.translate(reposition_vector * -other->getOwner()->getInvMass());
			}
			float vj = -(1.0f + e) * (collision_normal ^ relative_velocity);
			float j = vj / m_inv_sum;
			FVector ja = collision_normal * j * getOwner()->getInvMass();
			FVector jb = collision_normal * -j * other->getOwner()->getInvMass();
			getOwner()->applyImpulse(ja);
			other->getOwner()->applyImpulse(jb);
		}
		return true;
	}

	return false;
}
