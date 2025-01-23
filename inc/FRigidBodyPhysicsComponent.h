#pragma once

#include "FPhysicsComponent.h"
#include "FCollider.h"

class FRigidBodyPhysicsComponent : public FPhysicsComponent
{
private:
	FCollider* collider = nullptr;

public:
	using FPhysicsComponent::FPhysicsComponent;

	inline FCollider* getCollider() const override { return collider; }
	inline void setCollider(FCollider* col) override { collider = col; }

	inline bool isCollideable() const override { return collider != nullptr; }
};