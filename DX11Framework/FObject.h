#pragma once

#include <DirectXMath.h>
#include <vector>
#include <string>

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
	vector<FObject*> children;

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
	DirectX::XMFLOAT4X4 getTransform();
	FObject*& getParent();
	FObject* getChild(int i) const;
	int countChildren() const;
	void addChild(FObject* o);
	void removeChild(FObject* o);
};