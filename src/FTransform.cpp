#include "FTransform.h"

#include "FObject.h"

void FTransform::setLocalEuler(XMFLOAT3 e)
{
	XMStoreFloat4x4(&local_to_parent,
		XMMatrixIdentity()
		* XMMatrixScalingFromVector(XMLoadFloat3(&local_scale))
		* XMMatrixRotationX(XMConvertToRadians(e.x))
		* XMMatrixRotationY(XMConvertToRadians(e.y))
		* XMMatrixRotationZ(XMConvertToRadians(e.z))
		* XMMatrixTranslationFromVector(XMLoadFloat3(&local_position)));

	updateParamsFromLocal();
	updateWorldFromLocal();
}

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
	XMMATRIX rotation = XMMatrixRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(angle));
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
	if (parent == nullptr)
	{
		local_to_world = local_to_parent;
	}
	else
	{
		XMFLOAT4X4 parent_to_world = parent->getTransform();
		XMStoreFloat4x4(&local_to_world, XMLoadFloat4x4(&local_to_parent) * XMLoadFloat4x4(&parent_to_world));
	}

	propagate();
}

void FTransform::updateLocalFromWorld()
{
	if (parent == nullptr)
	{
		local_to_parent = local_to_world;
	}
	else
	{
		XMFLOAT4X4 parent_to_world = parent->getTransform();
		XMStoreFloat4x4(&local_to_parent, XMLoadFloat4x4(&local_to_world) * XMMatrixInverse(nullptr, XMLoadFloat4x4(&parent_to_world)));
	}

	propagate();
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
	if (XMVector3Length(q).m128_f32[0] < 0.0001f)
		local_quaternion = XMFLOAT4(0,0,1,0);
	else
		XMStoreFloat4(&local_quaternion, q);
	XMStoreFloat3(&local_scale, s);
}

void FTransform::propagate()
{
	for (FTransform* t : children)
		t->updateWorldFromLocal();
}

FTransform::FTransform(FTransform& other)
{
	local_to_parent = other.local_to_parent;
	local_to_world = other.local_to_world;
	local_position = other.local_position;
	local_quaternion = other.local_quaternion;
	local_scale = other.local_scale;

	parent = other.parent;
	children = other.children;
}

FTransform::FTransform(FTransform&& other) noexcept
{
	local_to_parent = other.local_to_parent;
	local_to_world = other.local_to_world;
	local_position = other.local_position;
	local_quaternion = other.local_quaternion;
	local_scale = other.local_scale;

	parent = other.parent;
	children = other.children;
}

FTransform FTransform::operator=(FTransform& other)
{
	local_to_parent = other.local_to_parent;
	local_to_world = other.local_to_world;
	local_position = other.local_position;
	local_quaternion = other.local_quaternion;
	local_scale = other.local_scale;

	parent = other.parent;
	children = other.children;

	return *this;
}

FTransform FTransform::operator=(FTransform&& other) noexcept
{
	local_to_parent = other.local_to_parent;
	local_to_world = other.local_to_world;
	local_position = other.local_position;
	local_quaternion = other.local_quaternion;
	local_scale = other.local_scale;

	parent = other.parent;
	children = other.children;

	return *this;
}

XMFLOAT3 FTransform::getPosition() const
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

XMFLOAT4 FTransform::getQuaternion() const
{
	XMVECTOR s; XMVECTOR q; XMVECTOR p;
	XMMatrixDecompose(&s, &q, &p, XMLoadFloat4x4(&local_to_world));
	XMFLOAT4 quat;
	if (XMVector3Length(q).m128_f32[0] < 0.0001f)
		quat = XMFLOAT4(0, 0, 1, 0);
	else
		XMStoreFloat4(&quat, q);
	return quat;
}

void FTransform::setQuaternion(XMFLOAT4 q)
{
	XMFLOAT3 p = getPosition();
	XMFLOAT3 s = getScale();

	XMStoreFloat4x4(&local_to_world,
		XMMatrixIdentity()
		* XMMatrixScalingFromVector(XMLoadFloat3(&s))
		* XMMatrixRotationQuaternion(XMLoadFloat4(&q))
		* XMMatrixTranslationFromVector(XMLoadFloat3(&p)));

	updateLocalFromWorld();
	updateParamsFromLocal();
}

FTransform* FTransform::getParent() const
{
	return parent;
}

int FTransform::countChildren() const
{
	return static_cast<int>(children.size());
}

void FTransform::addChild(FTransform* o)
{
	if (o == nullptr) return;
	if (children.count(o) > 0) return;
	if (o->parent != nullptr) return;

	children.insert(o);
	o->parent = this;
	o->updateWorldFromLocal();
}

void FTransform::removeChild(FTransform* o)
{
	if (o == nullptr) return;
	if (children.count(o) == 0) return;
	if (o->parent == nullptr) return;

	children.erase(o);
	o->parent = nullptr;
	o->updateWorldFromLocal();
}
