#pragma once

#include "FScene.h"

class MyScene : public FScene
{
private:
	FObject a;
	FObject b;

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};