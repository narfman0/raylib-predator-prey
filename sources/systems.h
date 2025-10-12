#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.h"

void spawnEntity(flecs::world &ecs, bool isPredator,
                 const Vector3 &parentPosition);
void updateTransform(TransformComponent &transform);
void updateEnergyComponent(flecs::world &ecs, flecs::entity &entity,
                           EnergyComponent &energyComponent);
void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator);