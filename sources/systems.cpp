#include "systems.h"

#include "components.h"
#include "globals.h"
#include "util.h"

void updateTransform(TransformComponent &transform) {
  transform.position =
      Vector3Add(transform.position, transform.velocity * GetFrameTime());
  if (transform.position.x > gridSizeHalfF) {
    transform.position.x = gridSizeHalfF;
    transform.velocity.x *= -1;
  }
  if (transform.position.z > gridSizeHalfF) {
    transform.position.z = gridSizeHalfF;
    transform.velocity.z *= -1;
  }
  if (transform.position.x < -gridSizeHalfF) {
    transform.position.x = -gridSizeHalfF;
    transform.velocity.x *= -1;
  }
  if (transform.position.z < -gridSizeHalfF) {
    transform.position.z = -gridSizeHalfF;
    transform.velocity.z *= -1;
  }
}

void spawnEntity(flecs::world &ecs, bool isPredator,
                 const Vector3 &parentPosition) {
  auto vel = Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)};
  auto entity = ecs.entity()
                    .set<TransformComponent>({parentPosition, vel})
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
    if (ecs.count<TransformComponent>() < maxEntities) {
      energyComponent.energy -= spawnEnergy * 0.5F;
      ecs.defer([&ecs, &e]() {
        spawnEntity(ecs, e.has<PredatorTag>(),
                    e.get<TransformComponent>().position);
      });
    }
  } else if (energyComponent.energy < 0) {
    ecs.defer([e] { e.destruct(); });
  }
}

void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator) {
  float dt = GetFrameTime();
  auto &transform = predator.get_mut<TransformComponent>();
  auto &energyComponent = predator.get_mut<EnergyComponent>();

  float minDistSq = pursuitRangeSq;
  flecs::entity closestPrey;
  Vector3 *closestPreyPosition = nullptr;
  ecs.query<PreyTag>().each([&](flecs::entity entity, PreyTag &) {
    auto &preyTransform = entity.get_mut<TransformComponent>();
    const float distSq =
        Vector3DistanceSqr(transform.position, preyTransform.position);
    if (distSq < minDistSq) {
      minDistSq = distSq;
      closestPrey = entity;
      closestPreyPosition = &preyTransform.position;
    }
  });

  if (closestPreyPosition != nullptr) {
    if (minDistSq > 1.0F) {
      Vector3 dir = Vector3Subtract(*closestPreyPosition, transform.position);
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
