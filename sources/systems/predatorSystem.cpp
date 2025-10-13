#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator) {
  float dt = GetFrameTime();
  auto &transform = predator.get_mut<TransformComponent>();
  auto &energyComponent = predator.get_mut<EnergyComponent>();

  float minDistSq = pursuitRangeSq;
  flecs::entity closestPrey;
  Vector3 closestPreyPosition;
  ecs.query<PreyTag>().each([&](flecs::entity entity, PreyTag &) {
    auto &preyTransform = entity.get<TransformComponent>();
    if (aabb(transform.position, pursuitRange, preyTransform.position)) {
      const float distSq =
          Vector3DistanceSqr(transform.position, preyTransform.position);
      if (distSq < minDistSq) {
        minDistSq = distSq;
        closestPrey = entity;
        closestPreyPosition = preyTransform.position;
      }
    }
  });

  if (closestPrey.is_valid()) {
    if (minDistSq > 1.0F) {
      Vector3 dir = Vector3Subtract(closestPreyPosition, transform.position);
      dir = Vector3Scale(Vector3Normalize(dir), speed);
      transform.velocity.x = dir.x;
      transform.velocity.z = dir.z;
    } else {
      ecs.defer([closestPrey] { closestPrey.destruct(); });
      energyComponent.energy += predatorEnergyGainAmount;
    }
  }
  if (energyComponent.energy < 0) {
    ecs.defer([predator] { predator.destruct(); });
  }
}
