#pragma once

#include "FComponent.h"
#include "FVector.h"
#include "FApplication.h"

class FPhysicsEngine;

class FPhysicsComponent : public FComponent
{ // TODO: thread safety
private:
	FVector accumulated_forces;
	FVector velocity;
	float mass = 1.0f;

private:
	inline FVector getAndClearAccumulator() { FVector copy = accumulated_forces; accumulated_forces = FVector(0, 0, 0); return copy; }

	FVector computeDragForce();
	FVector computeFrictionForce();

public:
	bool obeys_gravity = true;
	float drag_coefficient = 0.05f;
	float friction_coefficient = 0.3f;

public:
	using FComponent::FComponent;

	inline FComponentType getType() { return FComponentType::PHYSICS; }

	inline FPhysicsEngine* getEngine() { return FApplication::get()->getPhysics(); }

	inline FVector getVelocity() { return velocity; }
	inline void setVelocity(FVector v) { velocity = v; }

	inline float getMass() { return mass; }
	inline void setMass(float m) { mass = m; }

	inline void addForce(FVector f) { accumulated_forces = accumulated_forces + f; }

	inline virtual bool isCollideable() { return false; }

	virtual void tick(float delta);
};