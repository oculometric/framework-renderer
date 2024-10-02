#ifndef FOBJECT_H
#define FOBJECT_H

#include <DirectXMath.h>
#include <vector>

using namespace DirectX;
using namespace std;

enum FObjectType
{
	EMPTY,
	CAMERA,
	MESH
};

class FObject
{
private:
	XMFLOAT4X4 local_transform;
	XMFLOAT4X4 world_transform;

	FObject* parent;
	vector<FObject*> children; // TODO: make this be a reference to a resource manager which stores the children to solve all the memory deallocation problems from before

public:
	XMFLOAT3 position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 eulers = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

private:

public:
	FObject();

	inline virtual FObjectType getType() { return FObjectType::EMPTY; }

	void updateTransform();
	DirectX::XMFLOAT4X4 getTransform();
	FObject*& getParent();
	const FObject* getChild(int i);
	const int countChildren();
	void addChild(FObject* o);
	void removeChild(FObject* o);
};

#endif