#pragma once

#include "FScene.h"
#include "FMesh.h"
#include "FJsonParser.h"

class MyScene : public FScene
{
private:
	FObject* a;
	FObject* b;
	FObject* c;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};