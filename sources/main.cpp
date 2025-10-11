#include <list>
#include <vector>
#include <ctime>

#include "raylib.h"
#include <raymath.h>

int gridSize = 50;
float speed = 2.0f;
float maxEnergy = 10.0f;
float spawnFrequency = 10.0f;
float predatorEnergyLossFactor = 0.7f;

enum EntityType {PREDATOR, PREY};
struct Entity{
    Vector3 position;
    Vector3 velocity;
    EntityType type;
    float spawnTime = spawnFrequency;
    float energy = maxEnergy;
};

inline float randRange(float min, float max){
    return min + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(max-min)));
}

Entity spawnEntity(Vector3 pos, EntityType type){
    return Entity(Vector3{pos}, Vector3{randRange(-speed, speed), 0, randRange(-speed, speed)}, type);
}

void initializeEntities(std::vector<Entity>& entities, int count, float gridSizeHalfF) {
    for(int i=0; i<count; i++){
        Vector3 pos{randRange(-gridSizeHalfF, gridSizeHalfF), 0, randRange(-gridSizeHalfF, gridSizeHalfF)};
        Vector3 vel{randRange(-speed, speed), 0, randRange(-speed, speed)};
        EntityType type = i % 4 == 0 ? EntityType::PREDATOR : EntityType::PREY;
        entities.emplace_back(pos, vel, type);
    }
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
    initializeEntities(entities, gridSize, gridSizeHalfF);

    auto updateEntity = [&](Entity& entity, float dt, float gridSizeHalfF) {
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
    };

    auto handleSpawning = [&](Entity& entity, std::vector<Entity>& entities) {
        if (entity.spawnTime < 0 && entity.energy > maxEnergy * 0.5f) {
            Entity offspring = spawnEntity(entity.position, entity.type);
            offspring.spawnTime = spawnFrequency;
            entity.spawnTime = spawnFrequency;
            entities.push_back(offspring);
        } else {
            entity.spawnTime -= GetFrameTime();
        }
    };

    auto handlePredatorBehavior = [&](Entity& entity, std::vector<Entity>& entities, float dt) {
        if (entity.type != EntityType::PREDATOR) return;
        float minDist = 1e6f;
        int closestPreyIdx = -1;
        for (size_t j = 0; j < entities.size(); ++j) {
            if (entities[j].type == EntityType::PREY) {
                float dist = Vector3Distance(entity.position, entities[j].position);
                if (dist < minDist) {
                    minDist = dist;
                    closestPreyIdx = (int)j;
                }
            }
        }
        if (closestPreyIdx != -1) {
            Entity& prey = entities[closestPreyIdx];
            Vector3 dir = Vector3Subtract(prey.position, entity.position);
            float dist = Vector3Length(dir);
            if (dist > 1.0f) {
                dir = Vector3Scale(Vector3Normalize(dir), speed);
                entity.velocity.x = dir.x;
                entity.velocity.z = dir.z;
                entity.energy -= dt * predatorEnergyLossFactor;
            }else if (dist < 1.0f) {
                prey.energy = 0;
                entity.energy += maxEnergy * 0.5f;
            }
        }else{
            entity.energy -= dt * predatorEnergyLossFactor;
        }
    };

    auto removeDeadEntities = [&](std::vector<Entity>& entities) {
        for(auto it = entities.begin(); it != entities.end();) {
            if(it->energy <= 0){
                it = entities.erase(it);
            }else{
                ++it;
            }
        }
    };

    while (!WindowShouldClose()) {
        if(IsKeyPressed(KEY_R)) {
            entities.clear();
            initializeEntities(entities, gridSize, gridSizeHalfF);
        }
        float dt = GetFrameTime();
        for(int i=0; i<entities.size(); i++){
            updateEntity(entities[i], dt, gridSizeHalfF);
            handleSpawning(entities[i], entities);
            handlePredatorBehavior(entities[i], entities, dt);
        }
        removeDeadEntities(entities);

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