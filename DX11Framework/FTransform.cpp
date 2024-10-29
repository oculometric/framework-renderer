#include "FTransform.h"

#include "FObject.h"

XMFLOAT3 FTransform::getScale() const
{
	XMVECTOR s; XMVECTOR _1; XMVECTOR _2;
	XMMatrixDecompose(&s, &_1, &_2, XMLoadFloat4x4(&local_to_world));
	XMFLOAT3 world_scale; XMStoreFloat3(&world_scale, s);
	return world_scale;
}

void FTransform::translate(XMFLOAT3 v)
{
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&v));
	XMStoreFloat4x4(&local_to_world, XMLoadFloat4x4(&local_to_world) * translation);

	updateLocalFromWorld();
	updateParamsFromLocal();
}

void FTransform::rotate(XMFLOAT3 axis, float angle, XMFLOAT3 about)
{
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&about));
	XMMATRIX rotation = XMMatrixRotationAxis(XMLoadFloat3(&axis), angle);
	XMStoreFloat4x4(&local_to_world, XMLoadFloat4x4(&local_to_world) * XMMatrixInverse(nullptr, translation) * rotation * translation);

	updateLocalFromWorld();
	updateParamsFromLocal();
}

void FTransform::scale(XMFLOAT3 s, XMFLOAT3 about)
{
	XMMATRIX translation = XMMatrixTranslationFromVector(XMLoadFloat3(&about));
	XMMATRIX scale = XMMatrixScalingFromVector(XMLoadFloat3(&s));
	XMStoreFloat4x4(&local_to_world, XMLoadFloat4x4(&local_to_world) * XMMatrixInverse(nullptr, translation) * scale * translation);

	updateLocalFromWorld();
	updateParamsFromLocal();
}

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
}

// TODO: update children, somewhere

void FTransform::updateLocalFromWorld()
{
	FObject* parent = object->getParent();
	if (parent == nullptr)
	{
		local_to_parent = local_to_world;
	}
	else
	{
		XMFLOAT4X4 parent_to_world = parent->getTransform();
		XMStoreFloat4x4(&local_to_parent, XMLoadFloat4x4(&local_to_world) * XMMatrixInverse(nullptr, XMLoadFloat4x4(&parent_to_world)));
	}
}

void FTransform::updateLocalFromParams()
{
	XMStoreFloat4x4(&local_to_parent,
		XMMatrixIdentity()
		* XMMatrixScalingFromVector(XMLoadFloat3(&local_scale))
		* XMMatrixRotationQuaternion(XMLoadFloat4(&local_quaternion))
		* XMMatrixTranslationFromVector(XMLoadFloat3(&local_position)));
}

void FTransform::updateParamsFromLocal()
{
	XMVECTOR s; XMVECTOR q; XMVECTOR p;
	XMMatrixDecompose(&s, &q, &p, XMLoadFloat4x4(&local_to_parent));
	XMStoreFloat3(&local_position, p);
	XMStoreFloat4(&local_quaternion, q);
	XMStoreFloat3(&local_scale, s);
}

inline XMFLOAT3 FTransform::getPosition() const
{
	return XMFLOAT3(local_to_world._41, local_to_world._42, local_to_world._43);
}

void FTransform::setPosition(XMFLOAT3 p)
{
	local_to_world._41 = p.x;
	local_to_world._42 = p.y;
	local_to_world._43 = p.z;

	updateLocalFromWorld();
	updateParamsFromLocal();
}
