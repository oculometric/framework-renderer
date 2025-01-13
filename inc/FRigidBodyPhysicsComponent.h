#pragma once

#include "FPhysicsComponent.h";
#include "FCollider.h";

class FRigidBodyPhysicsComponent : public FPhysicsComponent
{
private:
	FCollider* collider = nullptr;

public:
	using FPhysicsComponent::FPhysicsComponent;

	inline FCollider* getCollider() { return collider; }
	inline void setCollider(FCollider* col) { collider = col; }

	inline bool isCollideable() override { return collider != nullptr; }
};