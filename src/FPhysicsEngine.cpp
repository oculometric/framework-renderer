#include "FPhysicsEngine.h"

#include "FDebug.h"
#include "FPhysicsComponent.h"

#include <thread>

#define AIR_DENSITY 1.29f
#define DRAG_COEFFICIENT 0.45f
#define LAMINAR_THRESHOLD 1.0f

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
	// TODO: tick all the physics objects etc
	if (!application->scene)
		return;

	for (FObject* obj : application->scene->all_objects)
	{
		FPhysicsComponent* comp = obj->getComponent<FPhysicsComponent>();
		if (!comp) continue;

		vector<FVector> forces = comp->getAndClearAccumulator();
		
		// apply gravitational force
		forces.push_back(gravity * comp->getMass());

		// apply drag force
		float s = 0.5f * AIR_DENSITY * DRAG_COEFFICIENT; // TODO: multiplied by cross sectional area
		FVector v = comp->getVelocity();
		float k = magnitude(v) > LAMINAR_THRESHOLD ? 2.0f : 1.0f;
		FVector f_d = v * -s * powf(magnitude(v), k);
		forces.push_back(f_d);

		// apply friction force
		// TODO: friction force

		FVector resultant_force = FVector(0, 0, 0);
		for (FVector f : forces) resultant_force = resultant_force + f;

		FVector acceleration = resultant_force / comp->getMass();

		FVector velocity = comp->getVelocity() + (acceleration * delta_time);
		comp->setVelocity(velocity);

		FVector translation = velocity * delta_time;

		// TODO: add an option to this function to leave children as they are (in world space)
		obj->transform.translate(translation);
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
