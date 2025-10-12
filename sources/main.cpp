#include <cstdlib>
#include <ctime>

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>

#include "components.h"
#include "systems.h"
#include "util.h"

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