#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void initializePredatorSystems(flecs::world &ecs) {
  ecs.system<const PredatorTag, TargetComponent, const Position>("Predator Targeting System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, const PredatorTag&, TargetComponent &TargetComponent,
                   const Position &position) {
        if (--TargetComponent.framesTillNextTargetSearch > 0) {
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
          TargetComponent.target = closestPrey;
          TargetComponent.targetDistanceSq = minDistSq;
          TargetComponent.targetPosition = closestPreyPosition;
        } else {
          TargetComponent.target = flecs::entity();
        }
        TargetComponent.framesTillNextTargetSearch =
            predatorTargetSearchCooldownFramesMin + rand() % 3;
      });
  ecs.system<const PredatorTag, const TargetComponent, const Position, Velocity, Energy>(
         "Predator Attack System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, const PredatorTag&,
                   const TargetComponent &TargetComponent,
                   const Position &position, Velocity &velocity,
                   Energy &energy) {
        if (TargetComponent.target.is_valid()) {
          if (TargetComponent.targetDistanceSq > entityDiameter) {
            Vector3 dir =
                Vector3Subtract(TargetComponent.targetPosition, position);
            dir = Vector3Scale(Vector3Normalize(dir), speed);
            velocity.x = dir.x;
            velocity.z = dir.z;
          } else {
            TargetComponent.target.destruct();
            energy.energy += predatorEnergyGainAmount;
          }
        }
      });
}
