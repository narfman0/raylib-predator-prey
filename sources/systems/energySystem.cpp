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

void updateEnergyComponent(flecs::world &ecs, flecs::entity &e,
                           EnergyComponent &energyComponent) {
  if (e.has<PredatorTag>()) {
    energyComponent.energy -= (GetFrameTime() * predatorEnergyLossFactor);
  } else {
    energyComponent.energy += (GetFrameTime() * preyEnergyGainFactor);
  }

  if (energyComponent.energy > spawnEnergy) {
    if (ecs.count<Position>() < maxEntities) {
      energyComponent.energy -= spawnEnergy * 0.5F;
      ecs.defer([&ecs, &e]() {
        spawnEntity(ecs, e.has<PredatorTag>(), e.get<Position>());
      });
    }
  } else if (energyComponent.energy < 0) {
    ecs.defer([e] { e.destruct(); });
  }
}