#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void spawnEntity(flecs::world &ecs, bool isPredator,
                 const Vector3 &parentPosition) {
  auto vel = Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)};
  auto entity = ecs.entity()
                    .set<Position>({parentPosition})
                    .set<Velocity>({vel})
                    .set<EnergyComponent>(
                        {randRange(spawnEnergy * 0.2F, spawnEnergy * 0.7F)});
  if (isPredator) {
    entity.add<PredatorTag>();
  } else {
    entity.add<PreyTag>();
  }
}

void initializeEnergySystems(flecs::world &ecs) {
  ecs.system<EnergyComponent>("Energy System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, EnergyComponent &energyComponent) {
        if (energyComponent.energy > spawnEnergy) {
          if (ecs.count<Position>() > maxEntities)
            return;
          energyComponent.energy -= spawnEnergy * 0.5F;
          ecs.defer([&ecs, &entity]() {
            spawnEntity(ecs, entity.has<PredatorTag>(), entity.get<Position>());
          });
        }
      });
  ecs.system<EnergyComponent>("No Energy System")
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, EnergyComponent &energyComponent) {
        if (energyComponent.energy < 0) {
          ecs.defer([entity] { entity.destruct(); });
        }
      });
  ecs.system<EnergyComponent, const PredatorTag>("Predator Energy System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, EnergyComponent &energyComponent,
                   const PredatorTag &) {
        energyComponent.energy -= (GetFrameTime() * predatorEnergyLossFactor);
      });
  ecs.system<EnergyComponent, const PreyTag>("Prey Energy System")
      .multi_threaded(true)
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity entity, EnergyComponent &energyComponent,
                   const PreyTag &) {
        energyComponent.energy += (GetFrameTime() * preyEnergyGainFactor);
      });
}