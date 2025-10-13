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
    entity.add<PredatorComponent>();
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
            spawnEntity(ecs, entity.has<PredatorComponent>(),
                        entity.get<Position>());
          });
        }
      });
  ecs.system<Energy>("No Energy System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, Energy &energy) {
        if (energy.energy < 0) {
          ecs.defer([entity] { entity.destruct(); });
        }
      });
  ecs.system<Energy, const PredatorComponent>("Predator Energy System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, Energy &energy,
                   const PredatorComponent &) {
        energy.energy -= (GetFrameTime() * predatorEnergyLossFactor);
      });
  ecs.system<Energy, const PreyTag>("Prey Energy System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, Energy &energy, const PreyTag &) {
        energy.energy += (GetFrameTime() * preyEnergyGainFactor);
      });
}