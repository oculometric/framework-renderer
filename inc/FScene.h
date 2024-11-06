#pragma once

#include <DirectXMath.h>
#include <string>
#include <unordered_set>
#include <array>

#include "FObject.h"
#include "FLight.h"

class FApplication;

using namespace DirectX;

struct FObjectPreload
{
	FObjectType object_type = FObjectType::EMPTY;
	std::string name = "";
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rotation = XMFLOAT3(0, 0, 0);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	std::vector<FObjectPreload> children;
	std::string data_name = "";
	std::string material_name = "";
	float float1 = 0;
	float float2 = 0;
	float float3 = 0;
	float float4 = 0;
	XMFLOAT3 colour = XMFLOAT3(0,0,0);
	float strength = 1;
	float angle = 45;
	bool cast_shadow = true;
};

class FScene
{
	friend class FGraphicsEngine;

public:
	FCamera* active_camera = nullptr;
	FObject* active_object = nullptr;
	std::string name = "Scene";
	XMFLOAT3 ambient_light = XMFLOAT3(0, 0, 0);
	float fog_start = 4.0f;
	float fog_end = 16.0f;
	float fog_strength = 0.6f;
	XMFLOAT3 fog_colour = XMFLOAT3(0.45f, 0.23f, 0.20f);

protected:
	std::unordered_set<FObject*> all_objects;
	std::vector<FLight*> all_lights;

	FObject root;
	FApplication* owner = nullptr;

private:
	std::vector<FObjectPreload> preload_array;

public:
	FScene() = delete;
	FScene(FApplication* application, std::string scene_file);
	FScene(FScene& other) = delete;
	FScene(FScene&& other) = delete;

	void addObject(FObject* o, FObject* parent);
	template <typename T>
	T* findObjectWithName(std::string name);

	void finalizePreload();
	inline void queueForPreload(FObjectPreload& o) { preload_array.push_back(o); }
	inline virtual void start() { }
	inline virtual void update(float delta_time) { (delta_time); }

private:
	void finalizeObject(FObjectPreload& o, FObject* parent);
};

template<typename T>
inline T* FScene::findObjectWithName(std::string name)
{
	for (FObject* obj : all_objects)
		if (obj->name == name)
			return (T*)obj;

	return nullptr;
}
