#pragma once

#include "FScene.h"
#include "FMesh.h"

class MyScene : public FScene
{
private:
	FMesh a;
	FMesh b;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};