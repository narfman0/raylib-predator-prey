#include <vector>
#include <ctime>

#include "raylib.h"
#include <raymath.h>

int gridSize = 50;
float speed = 2.0f;
float maxEnergy = 10.0f;

enum EntityType {PREDATOR, PREY};
struct Entity{
    Vector3 position;
    Vector3 velocity;
    EntityType type;
    float spawnTime;
    float energy;
};

inline float randRange(float min, float max){
    return min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
}

Entity spawnEntity(Vector3 pos, EntityType type){
    return Entity(Vector3{pos}, Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)}, type);
}

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

    float gridSizeF = (float)gridSize;
    float gridSizeHalfF = (float)gridSize/2.f;
    std::vector<Entity> entities;
    for(int i=0; i<gridSize; i++){
        Vector3 pos{randRange(-gridSizeHalfF, gridSizeHalfF), 0, randRange(-gridSizeHalfF, gridSizeHalfF)};
        Vector3 vel{randRange(-speed, speed), 0, randRange(-speed, speed)};
        EntityType type = i % 4 == 0 ? EntityType::PREDATOR : EntityType::PREY;
        entities.emplace_back(pos, vel, type);
    }

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        for(std::vector<Entity>::iterator it = entities.begin(); it != entities.end(); it++){
            Entity& entity = *it;
            entity.position = Vector3Add(entity.position, entity.velocity*dt);
            if(entity.position.x > gridSizeHalfF){
                entity.position.x = gridSizeHalfF;
                entity.velocity.x *= -1;
            }
            if(entity.position.z > gridSizeHalfF){
                entity.position.z = gridSizeHalfF;
                entity.velocity.z *= -1;
            }
            if(entity.position.x < -gridSizeHalfF){
                entity.position.x = -gridSizeHalfF;
                entity.velocity.x *= -1;
            }
            if(entity.position.z < -gridSizeHalfF){
                entity.position.z = -gridSizeHalfF;
                entity.velocity.z *= -1;
            }
            // TODO get closest non-type
            // TODO spawn offspring of same type
        }


        UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                for(Entity entity : entities){
                    DrawCube(entity.position, 1.0f, 1.0f, 1.0f, entity.type == EntityType::PREDATOR ? RED : GREEN);
                    DrawCubeWires(entity.position, 1.0f, 1.0f, 1.0f, entity.type == EntityType::PREDATOR ? MAROON : LIME);
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