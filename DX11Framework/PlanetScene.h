#pragma once

#include "FScene.h"

class PlanetScene : public FScene
{
private:
	FObject* ship_root;
	float speed = 1.0f;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};