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
		float force_magnitude = -modulus * (magnitude(a_to_b) - target_lengths[i / 2]);
		FVector force = (normalise(a_to_b) * force_magnitude) - (relative_velocity * damping);
		
		float length_after_force = magnitude(a_to_b) + ((force_magnitude / vertex_mass) * delta * delta);

		if (length_after_force > target_lengths[i / 2] * limit)
		{
			float limited_length = target_lengths[i / 2] * limit;
			float recomputed_force = ((limited_length - magnitude(a_to_b)) / (delta * delta / vertex_mass));
			force = (normalise(a_to_b) * recomputed_force) - (relative_velocity * damping);
		}
		else if (length_after_force < target_lengths[i / 2] / limit)
		{
			float limited_length = target_lengths[i / 2] / limit;
			float recomputed_force = ((limited_length - magnitude(a_to_b)) / (delta * delta / vertex_mass));
			force = (normalise(a_to_b) * recomputed_force) - (relative_velocity * damping);
		}

		FVector per_vert_force = force / ((fixed[vert_a] || fixed[vert_b]) ? 1.0f : 2.0f);
		force_accumulators[vert_a] = force_accumulators[vert_a] - per_vert_force;
		force_accumulators[vert_b] = force_accumulators[vert_b] + per_vert_force;
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
