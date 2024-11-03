#pragma once

#include "FScene.h"

class ShadowDemoScene : public FScene
{
private:

public:
	using FScene::FScene;

	void start();
	void update(float delta_time);
};