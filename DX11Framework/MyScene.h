#pragma once

#include "FScene.h"
#include "FMesh.h"
#include "FJsonParser.h"

class MyScene : public FScene
{
private:

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
	inline string getJsonPath() { return "MyScene.fscn"; }
};