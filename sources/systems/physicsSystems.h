#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.h"

// Initialize physics world and systems. Must be called before other systems that
// rely on PhysicsBody.
void initializePhysicsSystems(flecs::world &ecs);

// Helper to create a circle body for an entity. Returns the created b2Body*.
// Note: function implemented in physicsSystems.cpp and only declared here.
struct b2Body;
b2Body* createCircleBody(flecs::world &ecs, const Vector3 &position, float radius, bool isDynamic, flecs::entity owner);
