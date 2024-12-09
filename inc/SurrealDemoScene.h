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
	FObject* monitor;

	bool fly_mode = false;
	float current_speed = 0.0f;

	FObject* walk_cam;
	FObject* fly_cam;

	int interaction_mode = 0;
	XMFLOAT4X4 original_transform;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);

private:
	XMFLOAT2 getMouseDeltaAndReset();
	void selectUnderMouse();
};