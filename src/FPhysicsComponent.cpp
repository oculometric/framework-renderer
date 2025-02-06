#include "FPhysicsComponent.h"

#include "FPhysicsEngine.h"
#include "FDebug.h"

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
    if (collision_data.empty()) return total_vec;

    for (const auto& coll : collision_data)
    {
        FVector coll_n = coll.second;
        FVector normal_reaction_force = coll_n * -(coll_n ^ accumulated_forces);
        float friction_magnitude = magnitude(normal_reaction_force) * friction_coefficient;
        FVector friction_direction = normalise(-getVelocity());
        if (magnitude(getVelocity()) - (friction_magnitude * getInvMass()) < 0)
            friction_magnitude = magnitude(getVelocity());
        total_vec = total_vec + (friction_direction * friction_magnitude);
    }

    return total_vec;
}
