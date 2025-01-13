#pragma once

class FPhysicsComponent;

class FSphereCollider;
class FAABBCollider;

class FCollider abstract
{
	FPhysicsComponent* owner;
public:
	FCollider(FPhysicsComponent* _owner) { owner = _owner; }

	FPhysicsComponent* getOwner() { return owner; }

	virtual bool checkCollision(FCollider* other) = 0;
	virtual bool checkCollisionSphere(FSphereCollider* other) = 0;
	virtual bool checkCollisionBox(FAABBCollider* other) = 0;
};