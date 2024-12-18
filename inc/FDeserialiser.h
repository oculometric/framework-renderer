#pragma once

#include <DirectXMath.h>
#include <vector>

class FComponent;
class FObject;
class FScene;

bool operator>>(const FJsonElement& a, XMFLOAT3& other);
bool operator>>(const FJsonElement& a, XMFLOAT4& other);
bool operator>>(const FJsonElement& a, XMINT3& other);

bool deserialiseComponent(const FJsonElement& a, FComponent*& other, const FObject* object);
bool deserialiseObject(const FJsonElement& a, FObject*& other, const FScene* scene);
bool deserialiseScene(const FJsonElement& a, FScene* other);