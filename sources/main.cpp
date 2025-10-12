#include <cstdlib>
#include <ctime>

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>

#include "components.h"

int gridSize = 25;
float speed = 2.0f;
float predatorEnergyLossFactor = 0.7f;
float gridSizeF = (float)gridSize;
float gridSizeHalfF = (float)gridSize / 2.f;

inline float randRange(float min, float max) {
  return min + static_cast<float>(rand()) /
                   (static_cast<float>(RAND_MAX / (max - min)));
}

void initializeEntities(flecs::world &ecs, int count) {
  for (int i = 0; i < count; i++) {
    auto entity = ecs.entity()
        .set<TransformComponent>(
            {Vector3{randRange(-gridSizeHalfF, gridSizeHalfF), 0,
                     randRange(-gridSizeHalfF, gridSizeHalfF)},
             Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)}})
        .set<SpawnComponent>({spawnFrequency, maxEnergy});
    if (i % 4 == 0) {
      entity.add<PredatorTag>();
    } else {
      entity.add<PreyTag>();
    }
  }
}

void updateTransform(TransformComponent &transform) {
  transform.position = Vector3Add(transform.position, transform.velocity * GetFrameTime());
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
      spawnComponent.energy > maxEnergy * 0.5f) {
    spawnComponent.spawnTime = spawnFrequency;
    spawnComponent.energy -= maxEnergy * 0.5f;
    auto deferredSpawn = [&ecs, &e]()
    {
      auto entity = ecs.entity()
          .set<TransformComponent>({e.get<TransformComponent>().position, Vector3{randRange(-speed, speed), 0,
                                                                                  randRange(-speed, speed)}})
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
  TransformComponent &transform = predator.get_mut<TransformComponent>();
  SpawnComponent &spawn = predator.get_mut<SpawnComponent>();

  float minDist = 1e6f;
  flecs::entity closestPrey;
  Vector3 *closestPreyPosition = nullptr;
  ecs.query<PreyTag>().each([&](flecs::entity e, PreyTag &preyTag) {
    TransformComponent &preyTransform = e.get_mut<TransformComponent>();
    float dist = Vector3Distance(transform.position, preyTransform.position);
    if (dist < minDist) {
      minDist = dist;
      closestPrey = e;
      closestPreyPosition = &preyTransform.position;
    } });

  if (closestPreyPosition != nullptr) {
    Vector3 dir = Vector3Subtract(*closestPreyPosition, transform.position);
    float dist = Vector3Length(dir);
    if (dist > 1.0f) {
      dir = Vector3Scale(Vector3Normalize(dir), speed);
      transform.velocity.x = dir.x;
      transform.velocity.z = dir.z;
      spawn.energy -= dt * predatorEnergyLossFactor;
    } else if (dist < 1.0f) {
      ecs.defer([closestPrey]
                { closestPrey.destruct(); });
      spawn.energy += maxEnergy * 0.5f;
    }
  } else {
    spawn.energy -= dt * predatorEnergyLossFactor;
  }
  if (spawn.energy < 0) {
    ecs.defer([predator]
              { predator.destruct(); });
  }
}

int main(void) {
  const int screenWidth = 1600;
  const int screenHeight = 1200;
  InitWindow(screenWidth, screenHeight, "predator prey");
  DisableCursor();
  SetTargetFPS(60);

  flecs::world ecs;
  std::srand((unsigned int)std::time({}));
  initializeEntities(ecs, gridSize);

  ecs.system<SpawnComponent>()
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity e, SpawnComponent &spawnComponent)
            { updateSpawnComponent(ecs, e, spawnComponent); });
  ecs.system<TransformComponent>()
      .kind(flecs::OnUpdate)
      .each([](flecs::entity e, TransformComponent &transformComponent)
            { updateTransform(transformComponent); });
  ecs.system<PredatorTag>()
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity e, PredatorTag &tag)
            { updatePredatorBehavior(ecs, e); });

  Camera3D camera = {0};
  camera.position = Vector3{0.0f, 10.0f, 10.0f};
  camera.target = Vector3{0.0f, 0.0f, 0.0f};
  camera.up = Vector3{0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_R)) {
      ecs.reset();
      initializeEntities(ecs, gridSize);
    }
    ecs.progress(GetFrameTime());

    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    ecs.query<TransformComponent>().each(
        [&](flecs::entity e, TransformComponent &transformComponent) {
          bool isPredator = e.has<PredatorTag>();
          DrawCube(transformComponent.position, 1.0f, 1.0f, 1.0f,
                   isPredator ? RED : GREEN);
          DrawCubeWires(transformComponent.position, 1.0f, 1.0f, 1.0f,
                        isPredator ? MAROON : LIME);
        });

    DrawGrid(gridSize, 1.0f);
    EndMode3D();
    DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);
    DrawFPS(10, 10);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}