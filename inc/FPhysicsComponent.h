#pragma once

#include "FComponent.h"
#include "FVector.h"
#include "FApplication.h"
#include "FCollider.h"
#include "map"

class FPhysicsEngine;

class FPhysicsComponent : public FComponent
{ // TODO: thread safety
private:
	FVector accumulated_forces;
	FVector velocity;
	float mass = 1.0f;

	std::map<FPhysicsComponent*, FVector> collision_data;

private:
	inline FVector getAndClearAccumulator() { FVector copy = accumulated_forces; accumulated_forces = FVector(0, 0, 0); return copy; }

	FVector computeDragForce() const;
	FVector computeFrictionForce() const;

public:
	bool obeys_gravity = true;
	bool kinematic = false;
	float drag_coefficient = 0.05f;
	float friction_coefficient = 0.3f;
	float restitution = 0.8f;

public:
	using FComponent::FComponent;

	inline FComponentType getType() const override { return FComponentType::PHYSICS; }

	inline FPhysicsEngine* getEngine() const { return FApplication::get()->getPhysics(); }

	inline FVector getVelocity() const { return velocity; }
	inline void setVelocity(FVector v) { velocity = v; }

	inline float getMass() const { return mass; }
	inline float getInvMass() const { return kinematic ? 0.0f : (1.0f / mass); }
	inline void setMass(float m) { mass = m; }

	inline void addForce(FVector f) { accumulated_forces = accumulated_forces + f; }
	inline void applyImpulse(FVector i) { velocity = velocity + i; }

	inline virtual bool isCollideable() const { return false; }

	inline virtual FCollider* getCollider() const { return nullptr; }
	inline virtual void setCollider(FCollider* col) { }

	inline virtual bool beginColliding(FPhysicsComponent* other, FVector normal) { if (collision_data.contains(other)) return false; else collision_data.emplace(other, normal); return true; }
	inline virtual bool endColliding(FPhysicsComponent* other) { if (collision_data.contains(other)) { collision_data.erase(other); return true; } return false; }

	virtual void tick(float delta);
};