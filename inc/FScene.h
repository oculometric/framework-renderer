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
	string name = "";
	XMFLOAT3 position = XMFLOAT3(0, 0, 0);
	XMFLOAT3 rotation = XMFLOAT3(0, 0, 0);
	XMFLOAT3 scale = XMFLOAT3(1, 1, 1);
	vector<FObjectPreload> children;
	string data_name = "";
	string material_name = "";
	float float1 = 0;
	float float2 = 0;
	float float3 = 0;
	float float4 = 0;
	XMFLOAT3 colour = XMFLOAT3(0,0,0);
	float strength = 1;
	float angle = 45;
};

class FScene
{
	friend class FGraphicsEngine;

public:
	FCamera* active_camera = nullptr;
	FObject* active_object = nullptr;
	string name = "Scene";
	XMFLOAT3 ambient_light = XMFLOAT3(0, 0, 0);

protected:
	unordered_set<FObject*> all_objects;
	vector<FLight*> all_lights;

	FObject root;
	FApplication* owner = nullptr;

private:
	vector<FObjectPreload> preload_array;

public:
	FScene() = delete;
	FScene(FApplication* application, string scene_file);
	FScene(FScene& other) = delete;
	FScene(FScene&& other) = delete;

	void addObject(FObject* o, FObject* parent);
	template <typename T>
	T* findObjectWithName(string name);

	void finalizePreload();
	inline void queueForPreload(FObjectPreload& o) { preload_array.push_back(o); }
	inline virtual void start() { }
	inline virtual void update(float delta_time) { (delta_time); }

private:
	void finalizeObject(FObjectPreload& o, FObject* parent);
};

template<typename T>
inline T* FScene::findObjectWithName(string name)
{
	for (FObject* obj : all_objects)
		if (obj->name == name)
			return (T*)obj;

	return nullptr;
}