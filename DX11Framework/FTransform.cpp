#include "FTransform.h"

#include "FObject.h"

void FTransform::reset()
{
	local_position = XMFLOAT3(0,0,0);
	local_quaternion = XMFLOAT4(0,0,1,0);
	local_scale = XMFLOAT3(1,1,1);

	updateLocalFromParams();
	updateWorldFromLocal();
}

void FTransform::updateWorldFromLocal()
{
	FObject* parent = object->getParent();
	if (parent == nullptr)
	{
		local_to_world = local_to_parent;
	}
	else
	{
		XMFLOAT4X4 parent_to_world = parent->getTransform();
		XMStoreFloat4x4(&local_to_world, XMLoadFloat4x4(&local_to_parent) * XMLoadFloat4x4(&parent_to_world));
	}

	// TODO: update children
}

void FTransform::updateLocalFromParams()
{
	XMStoreFloat4x4(&local_to_parent,
		XMMatrixIdentity()
		* XMMatrixScalingFromVector(XMLoadFloat3(&local_scale))
		* XMMatrixRotationQuaternion(XMLoadFloat4(&local_quaternion))
		* XMMatrixTranslationFromVector(XMLoadFloat3(&local_position)));
}
