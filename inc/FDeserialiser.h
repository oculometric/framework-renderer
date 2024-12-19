#pragma once

#include <DirectXMath.h>
#include <vector>

#include "FJsonParser.h"

class FComponent;
class FObject;
class FScene;

bool operator>>(const FJsonElement& a, XMFLOAT3& other);
bool operator>>(const FJsonElement& a, XMFLOAT4& other);
bool operator>>(const FJsonElement& a, XMINT3& other);

bool deserialiseComponent(const FJsonElement& a, FComponent*& other, FObject* object);
bool deserialiseObject(const FJsonElement& a, FObject*& other, FScene* scene);
bool deserialiseScene(const FJsonElement& a, FScene* other);