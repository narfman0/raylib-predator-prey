#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.h"

void updateTransform(TransformComponent &transform);
void updateEnergyComponent(flecs::world &ecs, flecs::entity &entity,
                           EnergyComponent &energyComponent);
void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator);