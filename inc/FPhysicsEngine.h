#pragma once

#include "FApplication.h"

#include <chrono>
#include <thread>

class FPhysicsEngine
{
private:
	std::chrono::steady_clock::time_point last_frame_instant;
	std::thread physics_thread;
	bool stopping = false;

	FApplication* application;

public:
	FVector gravity = FVector(0, 0, -9.81);

private:
	float getDeltaTime();
	void resetTimer();
	void waitForNextFrame(float target_delta);

	void physicsTick(float delta_time);
	void physicsMain();

public:
	FPhysicsEngine(FApplication* owner);

	~FPhysicsEngine();
};