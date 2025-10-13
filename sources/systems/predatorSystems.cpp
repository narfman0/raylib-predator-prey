#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void initializePredatorSystems(flecs::world &ecs) {
  static flecs::query<const PreyTag> preyQuery;
  preyQuery = ecs.query<const PreyTag>();
  ecs.system<PredatorComponent, const Position>("Predator Targeting System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, PredatorComponent &predatorComponent,
                   const Position &position) {
        if (--predatorComponent.framesTillNextTargetSearch > 0) {
          return;
        }
        float minDistSq = pursuitRangeSq;
        flecs::entity closestPrey;
        Vector3 closestPreyPosition;
        preyQuery.each([&](flecs::entity entity, const PreyTag &) {
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
          predatorComponent.target = closestPrey;
          predatorComponent.targetDistanceSq = minDistSq;
          predatorComponent.targetPosition = closestPreyPosition;
        } else {
          predatorComponent.target = flecs::entity();
        }
        predatorComponent.framesTillNextTargetSearch =
            predatorTargetSearchCooldownFramesMin + rand() % 3;
      });
  ecs.system<const PredatorComponent, const Position, Velocity, Energy>(
         "Predator Attack System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity,
                   const PredatorComponent &predatorComponent,
                   const Position &position, Velocity &velocity,
                   Energy &energy) {
        if (predatorComponent.target.is_valid()) {
          if (predatorComponent.targetDistanceSq > entityWidth) {
            Vector3 dir =
                Vector3Subtract(predatorComponent.targetPosition, position);
            dir = Vector3Scale(Vector3Normalize(dir), speed);
            velocity.x = dir.x;
            velocity.z = dir.z;
          } else {
            predatorComponent.target.destruct();
            energy.energy += predatorEnergyGainAmount;
          }
        }
      });
}
