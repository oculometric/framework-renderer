#pragma once

#include "FScene.h"

class PhysicsScene : public FScene
{
private:
	FObject* cube_a;
	FObject* cube_b;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};