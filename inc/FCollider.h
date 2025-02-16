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

	virtual int checkCollision(FCollider* other) = 0;
	virtual int checkCollisionSphere(FSphereCollider* other) = 0;
	virtual int checkCollisionBox(FAABBCollider* other) = 0;
};