#pragma once

#include <flecs.h>
#include <raymath.h>

#include "globals.h"

struct PredatorComponent {
  flecs::entity target;
};

struct PreyTag {
  bool filler;
};

struct Position : Vector3 {};
struct Velocity : Vector3 {};

struct Energy {
  float energy = spawnEnergy / 2.0f;
};