#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void initializePredatorSystems(flecs::world &ecs) {
  ecs.system<const PredatorTag, TargetComponent, const Position>("Predator Targeting System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, const PredatorTag&, TargetComponent &targetComponent,
                   const Position &position) {
        if (--targetComponent.framesTillNextTargetSearch > 0) {
          return;
        }
        float minDistSq = pursuitRangeSq;
        flecs::entity closestPrey;
        Vector3 closestPreyPosition;
        ecs.query<const PreyTag>().each(
            [&](flecs::entity entity, const PreyTag &) {
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
          targetComponent.target = closestPrey;
          targetComponent.targetDistanceSq = minDistSq;
          targetComponent.targetPosition = closestPreyPosition;
        } else {
          targetComponent.target = flecs::entity();
        }
        targetComponent.framesTillNextTargetSearch =
            predatorTargetSearchCooldownFramesMin + rand() % 3;
      });
  ecs.system<const PredatorTag, const TargetComponent, const Position, Velocity, Energy>(
         "Predator Attack System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, const PredatorTag&,
                   const TargetComponent &targetComponent,
                   const Position &position, Velocity &velocity,
                   Energy &energy) {
        if (targetComponent.target.is_valid()) {
          if (targetComponent.targetDistanceSq > entityDiameter) {
            Vector3 dir =
                Vector3Subtract(targetComponent.targetPosition, position);
            dir = Vector3Scale(Vector3Normalize(dir), speed);
            velocity.x = dir.x;
            velocity.z = dir.z;
          } else {
            targetComponent.target.destruct();
            energy.energy += predatorEnergyGainAmount;
          }
        }
      });
}
