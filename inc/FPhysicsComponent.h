#pragma once

#include "FComponent.h"
#include "FVector.h"
#include "vector"

class FPhysicsComponent : public FComponent
{ // TODO: thread safety
private:
	std::vector<FVector> accumulated_forces;
	// TODO: angular velocity
	FVector velocity;
	float mass = 1.0f;

public:
	using FComponent::FComponent;

	inline FComponentType getType() { return FComponentType::PHYSICS; }

	inline std::vector<FVector> getAndClearAccumulator() { auto copy = accumulated_forces; accumulated_forces.clear(); return copy; }
	
	inline FVector getVelocity() { return velocity; }
	inline void setVelocity(FVector v) { velocity = v; }

	inline float getMass() { return mass; }
	inline void setMass(float m) { mass = m; }
};