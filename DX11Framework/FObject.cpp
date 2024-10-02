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
		* XMMatrixRotationZ(eulers.z) 
		* XMMatrixRotationX(eulers.x) 
		* XMMatrixRotationY(eulers.y) 
		* XMMatrixTranslation(position.x, position.y, position.z));

	if (parent)
	{
		XMFLOAT4X4 parent_transform = parent->getTransform();
		XMStoreFloat4x4(&world_transform, XMLoadFloat4x4(&parent_transform) * XMLoadFloat4x4(&local_transform));
	}
	else
		world_transform = local_transform;

	for (FObject* obj : children) updateTransform();
}

XMFLOAT4X4 FObject::getTransform()
{
	return world_transform;
}

FObject*& FObject::getParent()
{
	return parent;
}
