#pragma once

#include <vector>

#include "FPhysicsComponent.h"

class FConstrainedParticleSystemComponent : public FPhysicsComponent
{
public:
	float vertex_mass = 0.1f;
	float modulus = 0.8f;
	float damping = 0.2f;
	float limit = 1.2f;

private:
	std::vector<FVector> vertices;
	std::vector<FVector> velocities;
	std::vector<bool> fixed;
	std::vector<uint16_t> edges;
	std::vector<float> target_lengths;

public:
	using FPhysicsComponent::FPhysicsComponent;

	inline FComponentType getType() const override { return FComponentType::CONSTRAINED; }
	
	inline void addVertex(FVector v, bool _fixed)
	{
		vertices.push_back(v);
		velocities.push_back(FVector{ 0, 0, 0 });
		fixed.push_back(_fixed);
	}
	
	inline void addEdge(uint16_t first, uint16_t second, float target_length)
	{
		edges.push_back(first);
		edges.push_back(second);
		target_lengths.push_back(target_length);
	}

	inline std::vector<FVector> getVertices()
	{
		return vertices;
	}

	void tick(float delta) override;
};