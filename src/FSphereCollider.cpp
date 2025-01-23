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
		FVector collision_normal = normalise(global_origin_a - global_origin_b);
		FVector relative_velocity = getOwner()->getVelocity() - other->getOwner()->getVelocity();
		if ((collision_normal ^ relative_velocity) < 0.0f)
		{
			float e = other->getOwner()->restitution; if (getOwner()->restitution > e) e = getOwner()->restitution;
			float penetration_depth = magnitude(global_origin_a - global_origin_b) - (getRadius() + other->getRadius());
			if (penetration_depth < 0.0f)
			{
				FVector reposition_vector = collision_normal * penetration_depth;
				float total_mass = getOwner()->getMass() + other->getOwner()->getMass();
				getOwner()->getOwner()->transform.translate(reposition_vector * other->getOwner()->getMass() / total_mass);
				other->getOwner()->getOwner()->transform.translate(reposition_vector * -getOwner()->getMass() / total_mass);
			}
			float vj = -(1.0f + e) * (collision_normal ^ relative_velocity); // TODO: interpenetration
			float j = vj / ((1.0f / getOwner()->getMass()) + (1.0f / other->getOwner()->getMass()));
			FVector ja = collision_normal * j / getOwner()->getMass();
			FVector jb = collision_normal * -j / other->getOwner()->getMass();
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

	FVector closest_point = so;
	if (closest_point.x > bb.max_corner.x) closest_point.x = bb.max_corner.x;
	else if (closest_point.x < bb.min_corner.x) closest_point.x = bb.min_corner.x;
	if (closest_point.y > bb.max_corner.y) closest_point.y = bb.max_corner.y;
	else if (closest_point.y < bb.min_corner.y) closest_point.y = bb.min_corner.y;
	if (closest_point.z > bb.max_corner.z) closest_point.z = bb.max_corner.z;
	else if (closest_point.z < bb.min_corner.z) closest_point.z = bb.min_corner.z;

	float distance_squared = magnitude_squared(closest_point - so);

	return distance_squared < radius * radius;
}
