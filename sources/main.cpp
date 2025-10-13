#include <cstdlib>
#include <ctime>
#include <format>

#include "spdlog/spdlog.h"
#include <flecs.h>
#include <raylib.h>

#include "components.h"
#include "globals.h"
#include "systems/systems.h"
#include "util.h"

static void initializeEntities(flecs::world &ecs, int count, bool isPredator) {
  for (int i = 0; i < count; i++) {
    auto pos = Vector3{randRange(-gridSizeHalfF, gridSizeHalfF), 0,
                       randRange(-gridSizeHalfF, gridSizeHalfF)};
    spawnEntity(ecs, isPredator, pos);
  }
}

static void initialize(flecs::world &ecs) {
  spdlog::info("Initializing world...");
  initializeEntities(ecs, initialPredators, true);
  initializeEntities(ecs, initialPrey, false);
  initializeEnergySystems(ecs);
  initializeTransformSystems(ecs);
  initializePredatorSystems(ecs);
  ecs.set_threads(8);
  ecs.import <flecs::stats>();
  ecs.set<flecs::Rest>({});
  spdlog::info("World initialized!");
}

int main(void) {
  const int screenWidth = 1600;
  const int screenHeight = 1200;
  InitWindow(screenWidth, screenHeight, "predator prey");
  DisableCursor();
  SetTargetFPS(targetFps);

  flecs::world ecs;
  std::srand((unsigned int)std::time({}));
  initialize(ecs);

  Camera3D camera = {0};
  camera.position = Vector3{0.0F, 10.0F, 10.0F};
  camera.target = Vector3{0.0F, 0.0F, 0.0F};
  camera.up = Vector3{0.0F, 1.F, 0.0F};
  camera.fovy = 45.0F;
  camera.projection = CAMERA_PERSPECTIVE;

  spdlog::info("Initial setup complete, beginning game loop");

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_R)) {
      ecs.reset();
      initialize(ecs);
    }
    ecs.progress(GetFrameTime());

    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    ecs.query<Position>().each([&](flecs::entity entity, Position &position) {
      const bool isPredator = entity.has<PredatorComponent>();
      DrawCube(position, entityWidth, entityWidth, entityWidth,
               isPredator ? RED : GREEN);
      DrawCubeWires(position, entityWidth, entityWidth, entityWidth,
                    isPredator ? MAROON : LIME);
    });

    DrawGrid(gridSize, 1.0F);
    EndMode3D();
    DrawText(std::format("Entities: {}", ecs.count<Position>()).c_str(), 10, 40,
             20, DARKGRAY);
    DrawText(
        std::format("Predators: {}", ecs.count<PredatorComponent>()).c_str(),
        10, 80, 20, DARKGRAY);
    DrawText(std::format("Prey: {}", ecs.count<PreyTag>()).c_str(), 10, 120, 20,
             DARKGRAY);
    DrawText(std::format("Threads: {}", ecs.get_threads()).c_str(), 10, 160, 20,
             DARKGRAY);
    DrawFPS(10, 10);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}