#pragma once

#include <unordered_set>
#include <DirectXMath.h>

#include "FVector.h"

using namespace DirectX;

class FObject;

// class which encapsulates behaviour for maintaining a structured scene hierarchy, and implements numerous useful functions for applying transformations both in world space and local space
class FTransform
{
private:
	XMFLOAT4X4 local_to_parent;			// matrix which transforms vectors from model space to parent model space
	XMFLOAT4X4 local_to_world;			// matrix which transforms vectors from model space to world space

	FVector local_position;			// local position within parent space
	XMFLOAT4 local_quaternion;			// local rotation within parent space
	FVector local_scale;				// local scale    within parent space

	FTransform* parent = nullptr;
	std::unordered_set<FTransform*> children = { };

private:
	void updateWorldFromLocal();		// compute local-to-world matrix from local-to-parent matrix and the parent's local-to-world
	void updateLocalFromWorld();		// compute local-to-parent matrix from local-to-world matrix and the parent's local-to-world
	void updateLocalFromParams();		// compute local-to-parent matrix from individual local parameters
	void updateParamsFromLocal();		// compute individual local parameters from local-to-parent matrix
	
	void propagate();					// update child transforms
	
public:
	inline FTransform(FVector position, XMFLOAT4 quaternion = XMFLOAT4(0, 0, 1, 0), FVector scale = FVector(1, 1, 1))
		: local_position(position), local_quaternion(quaternion), local_scale(scale)
		{ updateLocalFromParams(); updateWorldFromLocal(); }

	inline FTransform(FVector position, FVector eulers, FVector scale = FVector(1, 1, 1))
		: local_position(position), local_scale(scale)
		{ setLocalEuler(eulers); }

	inline FTransform() 
		: local_position(FVector(0, 0, 0)), local_quaternion(XMFLOAT4(0, 0, 1, 0)), local_scale(FVector(1, 1, 1))
		{ local_to_world = local_to_parent = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); }

	FTransform(FTransform& other);
	FTransform(FTransform&& other) noexcept;
	FTransform operator=(FTransform& other);
	FTransform operator=(FTransform&& other) noexcept;

	inline XMFLOAT4X4 getTransform() const { return local_to_world; }
	inline XMFLOAT4X4 getLocalTransform() const { return local_to_parent; }
	void setTransform(XMFLOAT4X4 t);

	FVector getPosition() const;
	void setPosition(FVector p);
	inline FVector getLocalPosition() const { return local_position; }
	inline void setLocalPosition(FVector p) { local_position = p; updateLocalFromParams(); updateWorldFromLocal(); }

	XMFLOAT4 getQuaternion() const;
	void setQuaternion(XMFLOAT4 q);
	inline XMFLOAT4 getLocalQuaternion() const { return local_quaternion; }
	inline void setLocalQuaternion(XMFLOAT4 q) { local_quaternion = q; updateLocalFromParams(); updateWorldFromLocal(); }

	FVector getEuler() const;
	void setEuler(FVector e);
	FVector getLocalEuler() const;
	void setLocalEuler(FVector e);

	FVector getScale() const;
	inline FVector getLocalScale() const { return local_scale; }
	inline void setLocalScale(FVector s) { local_scale = s; updateLocalFromParams(); updateWorldFromLocal(); }

	void translate(FVector v);
	void rotate(FVector axis, float angle, FVector about);
	void scale(FVector s, FVector about);
	void faceForward(FVector forward, FVector up);
	void lookAt(FVector target, FVector eye, FVector up);
	void reset();

	inline FVector getRight() const { return FVector(local_to_world._11, local_to_world._12, local_to_world._13); }
	inline FVector getUp() const { return FVector(local_to_world._21, local_to_world._22, local_to_world._23); }
	inline FVector getForward() const { return FVector(local_to_world._31, local_to_world._32, local_to_world._33); }
	
	inline FVector getLocalRight() const { return FVector(local_to_parent._11, local_to_parent._12, local_to_parent._13); }
	inline FVector getLocalUp() const { return FVector(local_to_parent._21, local_to_parent._22, local_to_parent._23); }
	inline FVector getLocalForward() const { return FVector(local_to_parent._31, local_to_parent._32, local_to_parent._33); }

	FTransform* getParent() const;
	int countChildren() const;
	void addChild(FTransform* o);
	void removeChild(FTransform* o);
};