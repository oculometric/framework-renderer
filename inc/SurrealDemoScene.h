#pragma once

#include "FScene.h"

class SurrealDemoScene : public FScene
{
private:
	FObject* orrery_base;
	FObject* orrery_planet_a;
	FObject* orrery_planet_b;
	FObject* orrery_mid;
	FObject* orrery_core;

	bool fly_mode = false;
	float current_speed = 0.0f;

	FCamera* walk_cam;
	FCamera* fly_cam;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);

private:
	XMFLOAT2 getMouseDeltaAndReset();
	void selectUnderMouse();
};