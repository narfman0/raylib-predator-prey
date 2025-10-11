#include <vector>
#include <ctime>

#include "raylib.h"

enum EntityType {PREDATOR, PREY};
struct Entity{
    Vector3 position;
    Vector3 velocity;
    EntityType type;
};

int main(void)
{
    std::srand((unsigned int)std::time({}));
    const int screenWidth = 1600;
    const int screenHeight = 1200;

    InitWindow(screenWidth, screenHeight, "predator prey");

    Camera3D camera = { 0 };
    camera.position = Vector3{ 0.0f, 10.0f, 10.0f };
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };

    DisableCursor();
    SetTargetFPS(60);

    int gridSize = 100;
    std::vector<Entity> entities;
    for(int i=0; i<100; i++){
        Vector3 pos{rand() % gridSize - gridSize/2, 0, rand() % gridSize - gridSize/2};
        Vector3 vel{rand() % 3, 0, rand() % 3};
        EntityType type = i % 4 == 0 ? EntityType::PREDATOR : EntityType::PREY;
        entities.emplace_back(pos, vel, type);
    }

    while (!WindowShouldClose())
    {
        // TODO update pos


        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                for(Entity entity : entities){
                    DrawCube(entity.position, 2.0f, 2.0f, 2.0f, entity.type == EntityType::PREDATOR ? RED : GREEN);
                    DrawCubeWires(entity.position, 2.0f, 2.0f, 2.0f, entity.type == EntityType::PREDATOR ? MAROON : LIME);
                }
                DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
                DrawGrid(gridSize, 1.0f);
            EndMode3D();
            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);
            DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}