#pragma once

#include <flecs.h>
#include <raymath.h>

#include "globals.h"

struct TargetComponent {
  flecs::entity target;
  float targetDistanceSq = -1.0F;
  Vector3 targetPosition;
  int framesTillNextTargetSearch;
};

struct PredatorTag {
  bool filler;
};

struct PreyTag {
  bool filler;
};


struct Position : Vector3 {};
struct Velocity : Vector3 {};

struct Energy {
  float energy = spawnEnergy / 2.0f;
};