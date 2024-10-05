#include "FObject.h"

using namespace DirectX;

FObject::FObject()
{
	updateTransform();
}

void FObject::updateTransform()
{
	XMStoreFloat4x4(&local_transform, 
		XMMatrixIdentity() 
		* XMMatrixScaling(scale.x, scale.y, scale.z) 
		* XMMatrixRotationZ(XMConvertToRadians(eulers.z))
		* XMMatrixRotationX(XMConvertToRadians(eulers.x))
		* XMMatrixRotationY(XMConvertToRadians(eulers.y))
		* XMMatrixTranslation(position.x, position.y, position.z));

	if (parent)
	{
		XMFLOAT4X4 parent_transform = parent->getTransform();
		XMStoreFloat4x4(&world_transform, XMMatrixIdentity() * XMLoadFloat4x4(&local_transform) * XMLoadFloat4x4(&parent_transform));
	}
	else
		world_transform = local_transform;

	for (FObject* obj : children) obj->updateTransform();
}

XMFLOAT4X4 FObject::getTransform()
{
	return world_transform;
}

FObject*& FObject::getParent()
{
	return parent;
}

const FObject* FObject::getChild(int i)
{
	if (i < children.size()) return children[i];
	return nullptr;
}

const int FObject::countChildren()
{
	return children.size();
}

void FObject::addChild(FObject* o)
{
	// TODO: if child exists, abort
	// TODO: if child has a parent already, abort
	// TODO: if child is null, abort

	children.push_back(o);
	o->parent = this;
}
