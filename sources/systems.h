#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.h"
#include "util.h"

void updateTransform(TransformComponent &transform);
void updateSpawnComponent(flecs::world &ecs, flecs::entity &e,
                          SpawnComponent &spawnComponent);
void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator);