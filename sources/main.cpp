#include <ctime>
#include <vector>

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>

int gridSize = 25;
float speed = 2.0f;
float maxEnergy = 10.0f;
float spawnFrequency = 4.0f;
float predatorEnergyLossFactor = 0.7f;
float gridSizeF = (float)gridSize;
float gridSizeHalfF = (float)gridSize / 2.f;

enum EntityType { PREDATOR, PREY };

struct TransformComponent {
  Vector3 position;
  Vector3 velocity;
};
struct SpawnComponent {
  float spawnTime = spawnFrequency;
  float energy = maxEnergy;
};

inline float randRange(float min, float max) {
  return min + static_cast<float>(rand()) /
                   (static_cast<float>(RAND_MAX / (max - min)));
}

void initializeEntities(flecs::world &ecs, int count) {
  for (int i = 0; i < count; i++) {
    ecs.entity()
        .set<TransformComponent>(
            {Vector3{randRange(-gridSizeHalfF, gridSizeHalfF), 0,
                     randRange(-gridSizeHalfF, gridSizeHalfF)},
             Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)}})
        .set<EntityType>(i % 4 == 0 ? EntityType::PREDATOR : EntityType::PREY)
        .set<SpawnComponent>({spawnFrequency, maxEnergy});
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

void spawnEntity(flecs::world &ecs, EntityType type, Vector3 position) {
  ecs.entity()
      .set<TransformComponent>({position, Vector3{randRange(-speed, speed), 0,
                                                  randRange(-speed, speed)}})
      .set<EntityType>(type)
      .set<SpawnComponent>({spawnFrequency, maxEnergy});
}

void updateSpawnComponent(flecs::world &ecs, flecs::entity &e,
                          SpawnComponent &spawnComponent) {
  if (spawnComponent.spawnTime < 0 &&
      spawnComponent.energy > maxEnergy * 0.5f) {
    spawnComponent.spawnTime = spawnFrequency;
    spawnComponent.energy -= maxEnergy * 0.5f;
    auto deferredSpawn = [&ecs, &e]()
    {
      ecs.entity()
          .set<TransformComponent>({e.get<TransformComponent>().position, Vector3{randRange(-speed, speed), 0,
                                                                                  randRange(-speed, speed)}})
          .set<EntityType>(e.get<EntityType>())
          .set<SpawnComponent>({spawnFrequency, maxEnergy});
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
  flecs::entity *closestPrey = nullptr;
  Vector3 *closestPreyPosition = nullptr;
  ecs.query<EntityType>().each([&](flecs::entity e, EntityType &entityType){
    TransformComponent &preyTransform = e.get_mut<TransformComponent>();
    if (entityType == EntityType::PREY) {
      float dist = Vector3Distance(transform.position, preyTransform.position);
      if (dist < minDist) {
        minDist = dist;
        closestPrey = &e;
        closestPreyPosition = &preyTransform.position;
      }
    } });

  if (closestPrey != nullptr) {
    Vector3 dir = Vector3Subtract(*closestPreyPosition, transform.position);
    float dist = Vector3Length(dir);
    if (dist > 1.0f) {
      dir = Vector3Scale(Vector3Normalize(dir), speed);
      transform.velocity.x = dir.x;
      transform.velocity.z = dir.z;
      spawn.energy -= dt * predatorEnergyLossFactor;
    } else if (dist < 1.0f) {
      // ecs.defer([&closestPrey]()
      //           { closestPrey->destruct(); });
      spawn.energy += maxEnergy * 0.5f;
    }
  } else {
    spawn.energy -= dt * predatorEnergyLossFactor;
  }
}

int main(void) {
  flecs::world ecs;
  std::srand((unsigned int)std::time({}));
  const int screenWidth = 1600;
  const int screenHeight = 1200;

  InitWindow(screenWidth, screenHeight, "predator prey");

  Camera3D camera = {0};
  camera.position = Vector3{0.0f, 10.0f, 10.0f};
  camera.target = Vector3{0.0f, 0.0f, 0.0f};
  camera.up = Vector3{0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  DisableCursor();
  SetTargetFPS(60);

  initializeEntities(ecs, gridSize);

  ecs.system<SpawnComponent>()
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity e, SpawnComponent &spawnComponent)
            { updateSpawnComponent(ecs, e, spawnComponent); });
  ecs.system<TransformComponent>()
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity e, TransformComponent &transformComponent)
            { updateTransform(transformComponent); });
  ecs.system<EntityType>()
      .kind(flecs::OnUpdate)
      .each([&ecs](flecs::entity e, EntityType &entityType)
            { 
      if(entityType == EntityType::PREDATOR){
        updatePredatorBehavior(ecs, e);
      } });

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
          EntityType type = e.get<EntityType>();
          DrawCube(transformComponent.position, 1.0f, 1.0f, 1.0f,
                   type == EntityType::PREDATOR ? RED : GREEN);
          DrawCubeWires(transformComponent.position, 1.0f, 1.0f, 1.0f,
                        type == EntityType::PREDATOR ? MAROON : LIME);
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