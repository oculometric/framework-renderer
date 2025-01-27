#include "FPhysicsComponent.h"

#include "FPhysicsEngine.h"

#define AIR_DENSITY 1.29f
#define LAMINAR_THRESHOLD 1.0f

void FPhysicsComponent::tick(float delta)
{
    // apply gravitational force
    if (obeys_gravity) addForce(getEngine()->gravity * getMass());

    // apply drag force
    addForce(computeDragForce());

    // apply friction force
    addForce(computeFrictionForce());

    FVector total_force = getAndClearAccumulator();
    FVector acceleration = kinematic ? FVector(0, 0, 0) : total_force / getMass();

    setVelocity(getVelocity() + (acceleration * delta));

    FVector translation = getVelocity() * delta;

    // TODO: add an option to this function to leave children as they are (in world space)
    getOwner()->transform.translate(translation);
}

FVector FPhysicsComponent::computeDragForce() const
{
    float s = 0.5f * AIR_DENSITY * drag_coefficient; // TODO: multiplied by cross sectional area
    FVector v = getVelocity();
    float k = magnitude(v) > LAMINAR_THRESHOLD ? 2.0f : 1.0f;
    FVector f_d = v * -s * powf(magnitude(v), k);

    return f_d;
}

FVector FPhysicsComponent::computeFrictionForce() const
{
    FVector total_vec = FVector(0, 0, 0);
    return total_vec;

    // TODO: fix this (something fucked up)

    for (const auto& coll : collision_data)
    {
        FVector coll_n = coll.second;
        float f_mag = (coll_n ^ getVelocity()) * friction_coefficient;
        FVector f_v = getVelocity() - (coll_n * (coll_n ^ getVelocity()));
        if (magnitude_squared(f_v) < 0.001f) continue;
        FVector f_n = normalise(f_n);
        FVector f = f_n * -f_mag;
        total_vec = total_vec + f;
    }

    return total_vec;
}
