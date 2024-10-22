#pragma once

#include <DirectXMath.h>
#include <string>

#include "FObject.h"
#include "FCamera.h"

class FApplication;

using namespace DirectX;

class FScene
{
	friend class FApplication;

public:
	FCamera* active_camera = nullptr;
	string name = "Scene";

protected:
	vector<FObject*> all_objects;
	FObject root;
	FApplication* owner = nullptr;

public:
	FScene() = delete;
	inline FScene(FApplication* application) { owner = application; }
	FScene(FScene& other) = delete;
	FScene(FScene&& other) = delete;

	void addObject(FObject* o, FObject* parent);

	inline virtual void start() { }
	inline virtual void update(float delta_time) { (delta_time); }
};