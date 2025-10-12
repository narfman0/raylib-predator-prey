#include <raymath.h>

float maxEnergy = 10.0f;
float spawnFrequency = 4.0f;

struct PredatorTag
{
    bool filler;
};
struct PreyTag
{
    bool filler;
};
struct TransformComponent
{
    Vector3 position;
    Vector3 velocity;
};
struct SpawnComponent
{
    float spawnTime = spawnFrequency;
    float energy = maxEnergy;
};