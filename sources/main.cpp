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
  initializePhysicsSystems(ecs);
  initializeEntities(ecs, initialPredators, true);
  initializeEntities(ecs, initialPrey, false);
  initializeEnergySystems(ecs);
  initializeTransformSystems(ecs);
  initializePredatorSystems(ecs);
  ecs.set_threads(8);
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
  ecs.import <flecs::stats>();
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
      spdlog::info("Reinitialization complete");
    }
    ecs.progress(GetFrameTime());

    UpdateCamera(&camera, CAMERA_FIRST_PERSON);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode3D(camera);

    ecs.query<const Position, const PredatorTag &>().each(
        [&](flecs::entity entity, const Position &position,
            const PredatorTag &) {
          DrawCube(position, entityWidth, entityWidth, entityWidth, RED);
          DrawCubeWires(position, entityWidth, entityWidth, entityWidth,
                        MAROON);
        });
    ecs.query<const Position, const PreyTag &>().each(
        [&](flecs::entity entity, const Position &position, const PreyTag &) {
          DrawCube(position, entityWidth, entityWidth, entityWidth, GREEN);
          DrawCubeWires(position, entityWidth, entityWidth, entityWidth, LIME);
        });

    DrawGrid(gridSize, 1.0F);
    EndMode3D();
    DrawText(std::format("Entities: {}", ecs.count<Position>()).c_str(), 10, 40,
             20, DARKGRAY);
    DrawText(
        std::format("Predators: {}", ecs.count<PredatorTag>()).c_str(),
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