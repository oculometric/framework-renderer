#ifndef FOBJECT_H
#define FOBJECT_H

#include <DirectXMath.h>
#include <vector>

using namespace std;

class FObject
{
private:
	DirectX::XMFLOAT4X4 local_transform;
	DirectX::XMFLOAT4X4 world_transform;

	FObject* parent;
	vector<FObject*> children; // TODO: make this be a reference to a resource manager which stores the children to solve all the memory deallocation problems from before

public:
	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 eulers = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

private:

public:
	FObject();

	void updateTransform();
	DirectX::XMFLOAT4X4 getTransform();
	FObject*& getParent();
	const FObject* getChild(int i);
	const int countChildren();
	void addChild(FObject* o);
	void removeChild(FObject* o);
};

#endif