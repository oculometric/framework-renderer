#include "FConstrainedParticleSystemComponent.h"

#include "FPhysicsEngine.h"

using namespace std;

void FConstrainedParticleSystemComponent::tick(float delta)
{
	size_t num_vertices = vertices.size();
	vector<FVector> new_vertices(num_vertices);
	vector<FVector> new_velocities(num_vertices);
	vector<FVector> force_accumulators(num_vertices);

	for (size_t i = 0; i < edges.size() - 1; i += 2)
	{
		uint16_t vert_a = edges[i];
		uint16_t vert_b = edges[i + 1];

		if (fixed[vert_a] && fixed[vert_b])
			continue;

		FVector a_to_b = vertices[vert_b] - vertices[vert_a];
		FVector relative_velocity = velocities[vert_b] - velocities[vert_a];
		FVector force = (normalise(a_to_b) * (-modulus * (magnitude(a_to_b) - target_lengths[i / 2]))) - (relative_velocity * damping);
		float factor = 2.0f;
		if (fixed[vert_a] || fixed[vert_b])
			factor = 1.0f;
		force_accumulators[vert_a] = force_accumulators[vert_a] + (force / -factor);
		force_accumulators[vert_b] = force_accumulators[vert_b] + (force / factor);
	}

	for (size_t i = 0; i < num_vertices; i++)
	{
		if (fixed[i])
		{
			new_velocities[i] = FVector{ 0, 0, 0 };
			new_vertices[i] = vertices[i];
			continue;
		}
		force_accumulators[i] = force_accumulators[i] + (getEngine()->gravity * vertex_mass);
		FVector acceleration = force_accumulators[i] / vertex_mass;
		new_velocities[i] = velocities[i] + (acceleration * delta);
		new_vertices[i] = vertices[i] + (new_velocities[i] * delta);
	}

	vertices = new_vertices;
	velocities = new_velocities;
}
