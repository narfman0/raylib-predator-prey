#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator,
                            Position &position, Velocity &velocity,
                            EnergyComponent &energyComponent) {
  float dt = GetFrameTime();
  float minDistSq = pursuitRangeSq;
  flecs::entity closestPrey;
  Vector3 closestPreyPosition;
  ecs.query<const PreyTag>().each([&](flecs::entity entity, const PreyTag &) {
    auto &preyPosition = entity.get<Position>();
    if (aabb(position, pursuitRange, preyPosition)) {
      const float distSq = Vector3DistanceSqr(position, preyPosition);
      if (distSq < minDistSq) {
        minDistSq = distSq;
        closestPrey = entity;
        closestPreyPosition = preyPosition;
      }
    }
  });

  if (closestPrey.is_valid()) {
    if (minDistSq > 1.0F) {
      Vector3 dir = Vector3Subtract(closestPreyPosition, position);
      dir = Vector3Scale(Vector3Normalize(dir), speed);
      velocity.x = dir.x;
      velocity.z = dir.z;
    } else {
      ecs.defer([closestPrey] { closestPrey.destruct(); });
      energyComponent.energy += predatorEnergyGainAmount;
    }
  }
  if (energyComponent.energy < 0) {
    ecs.defer([predator] { predator.destruct(); });
  }
}

void initializePredatorSystems(flecs::world &ecs) {
  ecs.system<PredatorTag, Position, Velocity, EnergyComponent>(
         "Predator System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, PredatorTag &, Position &position,
                   Velocity &velocity, EnergyComponent &energyComponent) {
        updatePredatorBehavior(ecs, entity, position, velocity,
                               energyComponent);
      });
}
