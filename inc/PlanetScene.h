#pragma once

#include "FScene.h"

class PlanetScene : public FScene
{
private:
	FObject* ship_root;
	FObject* sun;

	float speed = 0.0f;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};