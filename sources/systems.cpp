#include "systems.h"

void updateTransform(TransformComponent &transform)
{
    transform.position = Vector3Add(transform.position, transform.velocity * GetFrameTime());
    if (transform.position.x > gridSizeHalfF)
    {
        transform.position.x = gridSizeHalfF;
        transform.velocity.x *= -1;
    }
    if (transform.position.z > gridSizeHalfF)
    {
        transform.position.z = gridSizeHalfF;
        transform.velocity.z *= -1;
    }
    if (transform.position.x < -gridSizeHalfF)
    {
        transform.position.x = -gridSizeHalfF;
        transform.velocity.x *= -1;
    }
    if (transform.position.z < -gridSizeHalfF)
    {
        transform.position.z = -gridSizeHalfF;
        transform.velocity.z *= -1;
    }
}

void updateSpawnComponent(flecs::world &ecs, flecs::entity &e,
                          SpawnComponent &spawnComponent)
{
    if (spawnComponent.spawnTime < 0 &&
        spawnComponent.energy > maxEnergy * 0.5f)
    {
        spawnComponent.spawnTime = spawnFrequency;
        spawnComponent.energy -= maxEnergy * 0.5f;
        auto deferredSpawn = [&ecs, &e]()
        {
            auto entity = ecs.entity()
                              .set<TransformComponent>({e.get<TransformComponent>().position, Vector3{randRange(-speed, speed), 0,
                                                                                                      randRange(-speed, speed)}})
                              .set<SpawnComponent>({spawnFrequency, maxEnergy});
            if (e.has<PredatorTag>())
            {
                entity.add<PredatorTag>();
            }
            else
            {
                entity.add<PreyTag>();
            }
        };
        ecs.defer(deferredSpawn);
    }
    else
    {
        spawnComponent.spawnTime -= GetFrameTime();
    }
}

void updatePredatorBehavior(flecs::world &ecs, flecs::entity &predator)
{
    float dt = GetFrameTime();
    TransformComponent &transform = predator.get_mut<TransformComponent>();
    SpawnComponent &spawn = predator.get_mut<SpawnComponent>();

    float minDist = 1e6f;
    flecs::entity closestPrey;
    Vector3 *closestPreyPosition = nullptr;
    ecs.query<PreyTag>().each([&](flecs::entity e, PreyTag &preyTag)
                              {
    TransformComponent &preyTransform = e.get_mut<TransformComponent>();
    float dist = Vector3Distance(transform.position, preyTransform.position);
    if (dist < minDist) {
      minDist = dist;
      closestPrey = e;
      closestPreyPosition = &preyTransform.position;
    } });

    if (closestPreyPosition != nullptr)
    {
        Vector3 dir = Vector3Subtract(*closestPreyPosition, transform.position);
        float dist = Vector3Length(dir);
        if (dist > 1.0f)
        {
            dir = Vector3Scale(Vector3Normalize(dir), speed);
            transform.velocity.x = dir.x;
            transform.velocity.z = dir.z;
            spawn.energy -= dt * predatorEnergyLossFactor;
        }
        else if (dist < 1.0f)
        {
            ecs.defer([closestPrey]
                      { closestPrey.destruct(); });
            spawn.energy += maxEnergy * 0.5f;
        }
    }
    else
    {
        spawn.energy -= dt * predatorEnergyLossFactor;
    }
    if (spawn.energy < 0)
    {
        ecs.defer([predator]
                  { predator.destruct(); });
    }
}
