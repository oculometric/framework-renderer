#include "FPhysicsEngine.h"

#include "FDebug.h"
#include "FPhysicsComponent.h"
#include "FConstrainedParticleSystemComponent.h"
#include "FDebug.h"

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

	vector<FPhysicsComponent*> comps;
	for (FObject* obj : application->scene->all_objects)
	{
		FPhysicsComponent* comp = obj->getComponent<FPhysicsComponent>();
		if (!comp) comp = obj->getComponent<FConstrainedParticleSystemComponent>();
		if (comp) comps.push_back(comp);
	}

	FDebug::get()->setComponentCount(comps.size());

	if (comps.size() == 0) return;

	for (int i = 0; i < comps.size() - 1; i++)
	{
		if (!comps[i]->isCollideable()) continue;

		for (int j = i + 1; j < comps.size(); j++)
		{
			if (!comps[j]->isCollideable()) continue;

			int status = comps[i]->getCollider()->checkCollision(comps[j]->getCollider());
			if (status != 0)
				FDebug::console("collision " + string((status == 1) ? "started" : "ended") + " between: " + comps[i]->getOwner()->name + " and " + comps[j]->getOwner()->name + "\n");
		}
	}

	for (FPhysicsComponent* comp : comps)
		comp->tick(delta_time);
}

void FPhysicsEngine::physicsMain()
{
	resetTimer();
	while (!stopping)
	{
		waitForNextFrame(1.0f / 60.0f);
		float dt = getDeltaTime();
		physicsTick(dt);
		FDebug::get()->setTickTime(getDeltaTime());
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
