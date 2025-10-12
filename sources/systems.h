#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.h"

void updateTransform(TransformComponent &transform);
void updateSpawnComponent(flecs::world &ecs, flecs::entity &entity,
                          SpawnComponent &spawnComponent);
void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator);