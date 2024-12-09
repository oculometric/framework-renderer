#pragma once

#include <DirectXMath.h>
#include <string>
#include <unordered_set>
#include <array>

#include "FObject.h"
#include "FLight.h"

class FApplication;

using namespace DirectX;

// represents preload parameters for an object, while being loaded from JSON. necessary due to the order in which objects, meshes, materials, textures, etc are loaded as resources
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

// encapsulates a scene environment
class FScene
{
	friend class FGraphicsEngine;

public:
	FCamera* active_camera = nullptr;						// camera which should be used for rendering
	FObject* active_object = nullptr;						// object which is currently active (selected, etc)
	std::string name = "Scene";								// name of the scene, useful for debugging and display
	XMFLOAT3 ambient_light = XMFLOAT3(0, 0, 0);				// ambient light colour for the scene world
	float fog_start = 4.0f;									// fog start distance for the scene world
	float fog_end = 16.0f;									// fog end distance for the scene world
	float fog_strength = 0.6f;								// fog strength factor for the scene world
	XMFLOAT3 fog_colour = XMFLOAT3(0.45f, 0.23f, 0.20f);	// colour of fog for the scene world

protected:
	std::unordered_set<FObject*> all_objects;				// list of all objects in the scene. order does not matter
	std::vector<FLight*> all_lights;						// list of all the scene lights

	FObject root;											// root object of the scene, which all objects are children of
	FApplication* owner = nullptr;							// owning application, used to access the application state and graphics engine

private:
	std::vector<FObjectPreload> preload_array;				// list of objects which have been deserialised from a JSON file and will be loaded for real when the scene initialises

public:
	FScene() = delete;
	FScene(FApplication* application, std::string scene_file);
	FScene(FScene& other) = delete;
	FScene(FScene&& other) = delete;

	void addObject(FObject* o, FObject* parent);			// add an object to the scene graph
	FObject* findObjectWithName(std::string name);			// find the first object within the scene graph, with the specified name

	void finalizePreload();									// realize all the objects present in the preload array
	inline void queueForPreload(FObjectPreload& o) { preload_array.push_back(o); }
	inline virtual void start() { }							// virtual method called when a scene starts, just after the preload array has been realized
	inline virtual void update(float delta_time) { (delta_time); } // virtual method called every time a frame is rendered

private:
	void finalizeObject(FObjectPreload& o, FObject* parent);// realize an object preload structure into an actual object and add it to the scene as appropriate
};

template<typename T>
inline T* FScene::findObjectWithName(std::string name)
{
	for (FObject* obj : all_objects)
		if (obj->name == name)
			return (T*)obj;

	return nullptr;
}
