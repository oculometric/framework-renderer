#include "FObject.h"

using namespace DirectX;

FObject::FObject()
{
	updateTransform();
}

// TODO: local and global transform functions

void FObject::updateTransform()
{
	// calculate the transform matrix relative to the parent (i.e. local transform from local offsets)
	XMStoreFloat4x4(&local_transform, 
		XMMatrixIdentity() 
		* XMMatrixScaling(scale.x, scale.y, scale.z) 
		* XMMatrixRotationY(XMConvertToRadians(eulers.y))
		* XMMatrixRotationX(XMConvertToRadians(eulers.x))
		* XMMatrixRotationZ(XMConvertToRadians(eulers.z))
		* XMMatrixTranslation(position.x, position.y, position.z));

	// update global transform using parent's global transform and our local one
	if (parent)
	{
		XMFLOAT4X4 parent_transform = parent->getTransform();
		XMStoreFloat4x4(&world_transform, XMMatrixIdentity() * XMLoadFloat4x4(&local_transform) * XMLoadFloat4x4(&parent_transform));
	}
	else
		world_transform = local_transform;

	// update all of our children
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
