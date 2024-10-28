#pragma once

#include <DirectXMath.h>
#include <string>
#include <unordered_set>

using namespace DirectX;
using namespace std;

enum FObjectType
{
	EMPTY,
	CAMERA,
	MESH,
	LIGHT
};

class FObject
{
protected:
	XMFLOAT4X4 local_transform;
	XMFLOAT4X4 world_transform;

	FObject* parent;
	unordered_set<FObject*> children;

public:
	string name = "Object";
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 eulers = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

private:

public:
	FObject();

	inline virtual FObjectType getType() { return FObjectType::EMPTY; }

	void updateTransform();
	XMFLOAT4X4 getTransform() const;
	FObject* getParent() const;
	int countChildren() const;
	void addChild(FObject* o);
	void removeChild(FObject* o);
};

class FCamera : public FObject
{
private:
	XMFLOAT4X4 projection_matrix = XMFLOAT4X4();

public:
	float aspect_ratio = 1.0f;
	float field_of_view = 90.0f;
	float near_clip = 0.01f;
	float far_clip = 100.0f;

public:
	inline FObjectType getType() { return FObjectType::CAMERA; }

	void lookAt(XMFLOAT3 eye, XMFLOAT3 target, XMFLOAT3 up);
	void updateProjectionMatrix();
	inline XMFLOAT4X4 getProjectionMatrix() const { return projection_matrix; }
};
