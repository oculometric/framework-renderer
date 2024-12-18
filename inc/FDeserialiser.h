#pragma once

#include <DirectXMath.h>
#include <vector>

class FComponent;
class FObject;
class FScene;

bool operator>>(const FJsonElement& a, XMFLOAT3& other);
bool operator>>(const FJsonElement& a, XMFLOAT4& other);
bool operator>>(const FJsonElement& a, XMINT3& other);

bool operator>>(const FJsonElement& a, FComponent*& other, const FObject* object);
bool operator>>(const FJsonElement& a, FObject*& other);
bool operator>>(const FJsonElement& a, FScene*& other);


// // necessary due to the order in which objects, meshes, materials, textures, etc are loaded as resources

// // represents preload parameters for a component, while being loaded from JSON
// struct FComponentPreload
// {
// 	FComponentType object_type = FComponentType::BLANK;
//     std::string data_name = "";
// 	std::string material_name = "";
// 	float float1 = 0;
// 	float float2 = 0;
// 	float float3 = 0;
// 	float float4 = 0;
// 	XMFLOAT3 colour = XMFLOAT3(0,0,0);
// 	float strength = 1;
// 	float angle = 45;
// 	bool cast_shadow = true;
// };

// // represents preload parameters for an object, while being loaded from JSON
// struct FObjectPreload
// {
// 	std::string name = "";
// 	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
// 	XMFLOAT3 rotation = XMFLOAT3(0, 0, 0);
// 	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
// 	std::vector<FObjectPreload> children;
//     std::vector<FComponentPreload> components;
// };