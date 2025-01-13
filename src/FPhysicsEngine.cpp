#include "FPhysicsEngine.h"

#include "FDebug.h"
#include "FPhysicsComponent.h"

#include <thread>

using namespace std;
using namespace chrono;

float FPhysicsEngine::getDeltaTime()
{
	return (duration<float>(steady_clock::now() - last_frame_instant)).count();
}

void FPhysicsEngine::resetTimer()
{
	last_frame_instant = steady_clock::now();
}

void FPhysicsEngine::waitForNextFrame(float target_delta)
{
	// why don't these fucking work????
	//this_thread::sleep_until(last_frame_instant + duration<float>(target_delta));
	//this_thread::sleep_for(duration<float>(target_delta - getDeltaTime()));

	while (getDeltaTime() < target_delta) { }
}

void FPhysicsEngine::physicsTick(float delta_time)
{
	if (!application->scene)
		return;

	for (FObject* obj : application->scene->all_objects)
	{
		FPhysicsComponent* comp = obj->getComponent<FPhysicsComponent>();
		if (!comp) continue;

		if (comp->isCollideable())
		{
			// TODO: actually test collisions
		}

		comp->tick(delta_time);
	}
}

void FPhysicsEngine::physicsMain()
{
	resetTimer();
	while (!stopping)
	{
		waitForNextFrame(1.0f / 60.0f);
		float dt = getDeltaTime();
		physicsTick(dt);
		FDebug::console("physics tick took " + to_string(getDeltaTime()) + "\n");
		resetTimer();
	}
}

FPhysicsEngine::FPhysicsEngine(FApplication* owner)
{
	application = owner;
	physics_thread = thread(&FPhysicsEngine::physicsMain, this);
}

FPhysicsEngine::~FPhysicsEngine()
{
	stopping = true;
	physics_thread.join();
}
