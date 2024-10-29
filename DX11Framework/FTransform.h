#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class FObject;

class FTransform
{
private:
	XMFLOAT4X4 local_to_parent;		// matrix which transforms vectors from model space to parent model space
	XMFLOAT4X4 local_to_world;		// matrix which transforms vectors from model space to world space

	XMFLOAT3 local_position;		// local position within parent space
	XMFLOAT4 local_quaternion;		// local rotation within parent space
	XMFLOAT3 local_scale;			// local scale    within parent space

	FObject* object = nullptr;

private:
	void updateWorldFromLocal();	// compute local-to-world matrix from local-to-parent matrix and the parent's local-to-world
	void updateLocalFromWorld();	// compute local-to-parent matrix from local-to-world matrix and the parent's local-to-world
	void updateLocalFromParams();	// compute local-to-parent matrix from individual local parameters
	void updateParamsFromLocal();	// compute individual local parameters from local-to-parent matrix

public:
	inline XMFLOAT4X4 getTransform() const { return local_to_world; }
	XMFLOAT4X4 getLocalTransform() const { return local_to_parent; }

	inline XMFLOAT3 getPosition() const;
	void setPosition(XMFLOAT3 p);
	XMFLOAT3 getLocalPosition() const { return local_position; }
	inline void setLocalPosition(XMFLOAT3 p) { local_position = p; updateLocalFromParams(); updateWorldFromLocal(); }

	XMFLOAT4 getQuaternion() const;
	void setQuaternion(XMFLOAT4 q);
	XMFLOAT4 getLocalQuaternion() const { return local_quaternion; }
	void setLocalQuaternion(XMFLOAT4 q) { local_quaternion = q; updateLocalFromParams(); updateWorldFromLocal(); }

	XMFLOAT3 getEuler() const;
	void setEuler(XMFLOAT3 e);
	XMFLOAT3 getLocalEuler() const;
	void setLocalEuler(XMFLOAT3 e);

	XMFLOAT3 getScale() const;
	XMFLOAT3 getLocalScale() const { return local_scale; }
	void setLocalScale(XMFLOAT3 s) { local_scale = s; updateLocalFromParams(); updateWorldFromLocal(); }

	void translate(XMFLOAT3 v);
	void rotate(XMFLOAT3 axis, float angle, XMFLOAT3 about);
	void scale(XMFLOAT3 s, XMFLOAT3 about);
	void faceForward(XMFLOAT3 forward, XMFLOAT3 up);
	void lookAt(XMFLOAT3 target, XMFLOAT3 eye, XMFLOAT3 up);
	void reset();

	XMFLOAT3 getRight() const { return XMFLOAT3(local_to_world._11, local_to_world._12, local_to_world._13); }
	XMFLOAT3 getUp() const { return XMFLOAT3(local_to_world._21, local_to_world._22, local_to_world._23); }
	XMFLOAT3 getForward() const { return XMFLOAT3(local_to_world._31, local_to_world._32, local_to_world._33); }

	XMFLOAT3 getLocalRight() const { return XMFLOAT3(local_to_parent._11, local_to_parent._12, local_to_parent._13); }
	XMFLOAT3 getLocalUp() const { return XMFLOAT3(local_to_parent._21, local_to_parent._22, local_to_parent._23); }
	XMFLOAT3 getLocalForward() const { return XMFLOAT3(local_to_parent._31, local_to_parent._32, local_to_parent._33); }
};