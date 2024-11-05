#pragma once

#include "FScene.h"

class SurrealDemoScene : public FScene
{
private:
	FObject* orrery_base;
	FObject* orrery_outer;
	FObject* orrery_planet_a;
	FObject* orrery_planet_b;
	FObject* orrery_inner;
	FObject* orrery_core;

	bool fly_mode = false;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};