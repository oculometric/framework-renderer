#include "FPhysicsComponent.h"

#define AIR_DENSITY 1.29f
#define LAMINAR_THRESHOLD 1.0f

void FPhysicsComponent::tick(float delta)
{
    FVector total_force = getAndClearAccumulator();
		
    // apply gravitational force
    total_force += getEngine()->gravity * getMass();

    // apply drag force
    total_force += computeDragForce();

    // apply friction force
    total_force += computeFrictionForce();

    FVector acceleration = total_force / getMass();

    FVector velocity = getVelocity() + (acceleration * delta_time);
    setVelocity(velocity);

    FVector translation = velocity * delta_time;

    // TODO: add an option to this function to leave children as they are (in world space)
    getOwner()->transform.translate(translation);
}

FVector FPhysicsComponent::computeDragForce()
{
    float s = 0.5f * AIR_DENSITY * drag_coefficient; // TODO: multiplied by cross sectional area
    FVector v = getVelocity();
    float k = magnitude(v) > LAMINAR_THRESHOLD ? 2.0f : 1.0f;
    FVector f_d = v * -s * powf(magnitude(v), k);

    return f_d;
}

FVector FPhysicsComponent::computeFrictionForce()
{
    return FVector(0,0,0);

    FVector coll_n; // TODO: find collision normal
    float f_mag = (coll_n ^ getVelocity()) * friction_coefficient;
    float f_n = normalise(getVelocity() - (coll_n * (coll_n ^ getVelocity())));
    FVector f = -f_mag * f_n;

    return f;
}
