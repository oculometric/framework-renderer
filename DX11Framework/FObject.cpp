#include "FObject.h"

#include "FJsonParser.h"

FObject::FObject()
{ updateTransform(); }

// TODO: local and global transform functions

void FObject::updateTransform()
{
	// calculate the transform matrix relative to the parent (i.e. local transform from local offsets)
	XMStoreFloat4x4(&local_transform, 
		XMMatrixIdentity() 
		* XMMatrixScaling(scale.x, scale.y, scale.z) 
		* XMMatrixRotationY(XMConvertToRadians(eulers.y))
		* XMMatrixRotationX(XMConvertToRadians(-eulers.x))
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

XMFLOAT4X4 FObject::getTransform() const
{ return world_transform; }

FObject* FObject::getParent() const
{ return parent; }

int FObject::countChildren() const
{ return static_cast<int>(children.size()); }

void FObject::addChild(FObject* o)
{
	if (o == nullptr) return;
	if (children.count(o) > 0) return;
	if (o->parent != nullptr) return;

	children.insert(o);
	o->parent = this;
}

void FObject::removeChild(FObject* o)
{
	if (o == nullptr) return;
	if (children.count(o) == 0) return;
	if (o->parent == nullptr) return;

	children.erase(o);
	o->parent = nullptr;
}
