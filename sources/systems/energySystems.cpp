#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void spawnEntity(flecs::world &ecs, bool isPredator,
                 const Vector3 &parentPosition) {
  auto vel = Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)};
  auto entity =
      ecs.entity()
          .set<Position>({parentPosition})
          .set<Velocity>({vel})
          .set<Energy>({randRange(spawnEnergy * 0.2F, spawnEnergy * 0.7F)});
  if (isPredator) {
    entity.add<PredatorTag>();
    entity.add<TargetComponent>();
  } else {
    entity.add<PreyTag>();
  }
}

void initializeEnergySystems(flecs::world &ecs) {
  ecs.system<Energy>("Energy System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, Energy &energy) {
        if (energy.energy > spawnEnergy) {
          if (ecs.count<Position>() > maxEntities)
            return;
          energy.energy -= spawnEnergy * 0.5F;
          ecs.defer([&ecs, &entity]() {
            spawnEntity(ecs, entity.has<PredatorTag>(),
                        entity.get<Position>());
          });
        }
      });
  ecs.system<const Energy>("No Energy System")
      .multi_threaded(true)
      .kind(flecs::PostUpdate)
      .each([&ecs](flecs::entity entity, const Energy &energy) {
        if (energy.energy < 0) {
          entity.destruct();
        }
      });
  ecs.system<Energy, const PredatorTag>("Predator Energy System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, Energy &energy,
                   const PredatorTag &) {
        energy.energy -= (GetFrameTime() * predatorEnergyLossFactor);
      });
  ecs.system<Energy, const PreyTag>("Prey Energy System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, Energy &energy, const PreyTag &) {
        energy.energy += (GetFrameTime() * preyEnergyGainFactor);
      });
}