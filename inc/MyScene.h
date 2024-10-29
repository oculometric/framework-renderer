#pragma once

#include "FScene.h"
#include "FMesh.h"
#include "FJsonParser.h"

class MyScene : public FScene
{
private:
	FMesh* a;
	FMesh* b;
	FMesh* c;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};