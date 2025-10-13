#pragma once

#include <flecs.h>
#include <raylib.h>

#include "components.h"

void spawnEntity(flecs::world &ecs, bool isPredator,
                 const Vector3 &parentPosition);
void updateTransform(Position &position, Velocity &velocity);
void updateEnergyComponent(flecs::world &ecs, flecs::entity &entity,
                           EnergyComponent &energyComponent);
void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator,
                            Position &position, Velocity &velocity,
                            EnergyComponent &energyComponent);