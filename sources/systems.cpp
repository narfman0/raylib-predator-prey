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

void updateSpawnComponent(flecs::world &ecs, flecs::entity &e,
                          SpawnComponent &spawnComponent) {
  if (spawnComponent.spawnTime < 0 &&
      spawnComponent.energy > maxEnergy * 0.5F) {
    spawnComponent.spawnTime = spawnFrequency;
    spawnComponent.energy -= maxEnergy * 0.5F;
    auto deferredSpawn = [&ecs, &e]() {
      auto pos = e.get<TransformComponent>().position;
      auto vel = Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)};
      auto entity = ecs.entity()
                        .set<TransformComponent>({pos, vel})
                        .set<SpawnComponent>({spawnFrequency, maxEnergy});
      if (e.has<PredatorTag>()) {
        entity.add<PredatorTag>();
      } else {
        entity.add<PreyTag>();
      }
    };
    ecs.defer(deferredSpawn);
  } else {
    spawnComponent.spawnTime -= GetFrameTime();
  }
}

void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator) {
  float dt = GetFrameTime();
  auto &transform = predator.get_mut<TransformComponent>();
  auto &spawn = predator.get_mut<SpawnComponent>();

  float minDist = 1e6F;
  flecs::entity closestPrey;
  Vector3 *closestPreyPosition = nullptr;
  ecs.query<PreyTag>().each([&](flecs::entity entity, PreyTag &) {
    auto &preyTransform = entity.get_mut<TransformComponent>();
    const float dist =
        Vector3Distance(transform.position, preyTransform.position);
    if (dist < minDist) {
      minDist = dist;
      closestPrey = entity;
      closestPreyPosition = &preyTransform.position;
    }
  });

  if (closestPreyPosition != nullptr) {
    if (minDist > 1.0F) {
      Vector3 dir = Vector3Subtract(*closestPreyPosition, transform.position);
      dir = Vector3Scale(Vector3Normalize(dir), speed);
      transform.velocity.x = dir.x;
      transform.velocity.z = dir.z;
      spawn.energy -= dt * predatorEnergyLossFactor;
    } else {
      ecs.defer([closestPrey] { closestPrey.destruct(); });
      spawn.energy += maxEnergy * 0.5F;
    }
  } else {
    spawn.energy -= dt * predatorEnergyLossFactor;
  }
  if (spawn.energy < 0) {
    ecs.defer([predator] { predator.destruct(); });
  }
}
