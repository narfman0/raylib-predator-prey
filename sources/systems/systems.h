#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.h"

void spawnEntity(flecs::world &ecs, bool isPredator,
                 const Vector3 &parentPosition);
void initializeTransformSystems(flecs::world &ecs);
void initializeEnergySystems(flecs::world &ecs);
void initializePredatorSystems(flecs::world &ecs);
void initializePhysicsSystems(flecs::world &ecs);